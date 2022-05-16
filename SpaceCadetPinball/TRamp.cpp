#include "pch.h"
#include "TRamp.h"


#include "control.h"
#include "loader.h"
#include "TBall.h"
#include "TEdgeSegment.h"
#include "TLine.h"
#include "TPinballTable.h"
#include "TTableLayer.h"


TRamp::TRamp(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
{
	visualStruct visual{};
	vector2 wall1End{}, wall1Start{}, wall2Start{}, wall2End{};

	MessageField = 0;
	UnusedBaseFlag = 1;
	loader::query_visual(groupIndex, 0, &visual);
	CollisionGroup = visual.CollisionGroup;

	BallFieldMult = loader::query_float_attribute(groupIndex, 0, 701, 0.2f);
	BallZOffsetFlag = static_cast<int>(loader::query_float_attribute(groupIndex, 0, 1305, 0));

	auto floatArr3Plane = loader::query_float_attribute(groupIndex, 0, 1300);
	RampPlaneCount = static_cast<int>(floor(*floatArr3Plane));
	RampPlane = reinterpret_cast<ramp_plane_type*>(floatArr3Plane + 1);

	auto wall0Arr = loader::query_float_attribute(groupIndex, 0, 1303);
	auto wall0CollisionGroup = 1 << static_cast<int>(floor(wall0Arr[0]));
	auto wall0Pts = reinterpret_cast<wall_point_type*>(wall0Arr + 2);
	Line1 = new TLine(this, &ActiveFlag, wall0CollisionGroup, wall0Pts->Pt1, wall0Pts->Pt0);
	Line1->WallValue = nullptr;
	Line1->place_in_grid();
	EdgeList.push_back(Line1);

	auto wall1Arr = loader::query_float_attribute(groupIndex, 0, 1301);
	Wall1CollisionGroup = 1 << static_cast<int>(floor(wall1Arr[0]));
	auto wall1Enabled = static_cast<int>(floor(wall1Arr[1]));
	Wall1BallOffset = wall1Arr[7];
	maths::find_closest_edge(
		RampPlane,
		RampPlaneCount,
		reinterpret_cast<wall_point_type*>(wall1Arr + 3),
		wall1End,
		wall1Start);

	Line2 = new TLine(this, &ActiveFlag, CollisionGroup, wall1Start, wall1End);
	Line2->WallValue = nullptr;
	Line2->place_in_grid();
	EdgeList.push_back(Line2);

	auto wall2Arr = loader::query_float_attribute(groupIndex, 0, 1302);
	Wall2CollisionGroup = 1 << static_cast<int>(floor(wall2Arr[0]));
	auto wall2Enabled = static_cast<int>(floor(wall2Arr[1]));
	Wall2BallOffset = wall2Arr[7];
	maths::find_closest_edge(
		RampPlane,
		RampPlaneCount,
		reinterpret_cast<wall_point_type*>(wall2Arr + 3),
		wall2End,
		wall2Start);

	Line3 = new TLine(this, &ActiveFlag, CollisionGroup, wall2Start, wall2End);
	Line3->WallValue = nullptr;
	Line3->place_in_grid();
	EdgeList.push_back(Line3);


	auto xMin = 1000000000.0f;
	auto yMin = 1000000000.0f;
	auto yMax = -1000000000.0f;
	auto xMax = -1000000000.0f;
	for (auto index = 0; index < RampPlaneCount; index++)
	{
		auto& plane = RampPlane[index];
		vector2* pointOrder[4] = { &plane.V1, &plane.V2, &plane.V3, &plane.V1 };

		xMin = std::min(std::min(std::min(plane.V3.X, plane.V1.X), plane.V2.X), xMin);
		yMin = std::min(std::min(std::min(plane.V3.Y, plane.V1.Y), plane.V2.Y), xMin); // Sic
		xMax = std::max(std::max(std::max(plane.V3.X, plane.V1.X), plane.V2.X), xMin);
		yMax = std::max(std::max(std::max(plane.V3.Y, plane.V1.Y), plane.V2.Y), xMin);
		
		for (auto pt = 0; pt < 3; pt++)
		{
			auto& point1 = *pointOrder[pt], point2 = *pointOrder[pt + 1];
			auto collisionGroup = 0;

			if (point1 == wall1End && point2 == wall1Start)
			{
				if (wall1Enabled)
					collisionGroup = Wall1CollisionGroup;
			}
			else if (point1 == wall2End && point2 == wall2Start)
			{
				if (wall2Enabled)
					collisionGroup = Wall2CollisionGroup;
			}
			else
				collisionGroup = visual.CollisionGroup;

			if (collisionGroup)
			{
				auto line = new TLine(this, &ActiveFlag, collisionGroup, point1, point2);
				line->WallValue = &plane;
				line->place_in_grid();
				EdgeList.push_back(line);
			}
		}

		plane.FieldForce.X = cos(plane.GravityAngle2) * sin(plane.GravityAngle1) *
			PinballTable->GravityDirVectMult;
		plane.FieldForce.Y = sin(plane.GravityAngle2) * sin(plane.GravityAngle1) *
			PinballTable->GravityDirVectMult;
	}

	Field.Flag2Ptr = &ActiveFlag;
	Field.CollisionComp = this;
	Field.Mask = visual.CollisionGroup;

	auto x1 = xMax;
	auto y1 = yMax;
	auto x0 = xMin;
	auto y0 = yMin;
	TTableLayer::edges_insert_square(y0, x0, y1, x1, nullptr, &Field);
}

void TRamp::put_scoring(int index, int score)
{
	if (index < 4)
		Scores[index] = score;
}

int TRamp::get_scoring(int index)
{
	return index < 4 ? Scores[index] : 0;
}

void TRamp::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float coef, TEdgeSegment* edge)
{
	ball->not_again(edge);
	ball->Position.X = nextPosition->X;
	ball->Position.Y = nextPosition->Y;
	ball->RayMaxDistance -= coef;

	auto plane = static_cast<ramp_plane_type*>(edge->WallValue);
	if (plane)
	{
		ball->CollisionFlag = 1;
		ball->CollisionOffset.X = plane->BallCollisionOffset.X;
		ball->CollisionOffset.Y = plane->BallCollisionOffset.Y;
		ball->CollisionOffset.Z = plane->BallCollisionOffset.Z;
		ball->RampFieldForce.X = plane->FieldForce.X;
		ball->RampFieldForce.Y = plane->FieldForce.Y;
		ball->Position.Z = ball->Position.X * ball->CollisionOffset.X + ball->Position.Y * ball->CollisionOffset.Y +
			ball->Offset + ball->CollisionOffset.Z;
		ball->FieldFlag = CollisionGroup;
		return;
	}

	if (edge == Line1)
	{
		if (!PinballTable->TiltLockFlag)
		{
			loader::play_sound(SoftHitSoundId);
			control::handler(63, this);
		}
	}
	else
	{
		ball->CollisionFlag = 0;
		if (edge == Line2)
		{
			ball->FieldFlag = Wall1CollisionGroup;
			if (BallZOffsetFlag)
				ball->Position.Z = ball->Offset + Wall1BallOffset;
		}
		else
		{
			ball->FieldFlag = Wall2CollisionGroup;
			if (BallZOffsetFlag)
				ball->Position.Z = ball->Offset + Wall2BallOffset;
		}
	}
}

int TRamp::FieldEffect(TBall* ball, vector2* vecDst)
{
	vecDst->X = ball->RampFieldForce.X - ball->Acceleration.X * ball->Speed * BallFieldMult;
	vecDst->Y = ball->RampFieldForce.Y - ball->Acceleration.Y * ball->Speed * BallFieldMult;
	return 1;
}

void TRamp::port_draw()
{
	TCollisionComponent::port_draw();
}
