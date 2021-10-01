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
	vector_type end{}, start{}, *end2, *start2, *start3, *end3;

	MessageField = 0;
	UnusedBaseFlag = 1;
	loader::query_visual(groupIndex, 0, &visual);
	CollisionGroup = visual.CollisionGroup;

	BallFieldMult = loader::query_float_attribute(groupIndex, 0, 701, 0.2f);
	RampFlag1 = static_cast<int>(loader::query_float_attribute(groupIndex, 0, 1305, 0));

	auto floatArr3Plane = loader::query_float_attribute(groupIndex, 0, 1300);
	RampPlaneCount = static_cast<int>(floor(*floatArr3Plane));
	RampPlane = reinterpret_cast<ramp_plane_type*>(floatArr3Plane + 1);

	auto floatArr4 = loader::query_float_attribute(groupIndex, 0, 1303);
	end.X = floatArr4[2];
	end.Y = floatArr4[3];
	start.X = floatArr4[4];
	start.Y = floatArr4[5];
	Line1 = new TLine(this, &ActiveFlag, 1 << static_cast<int>(floor(floatArr4[0])), &start, &end);
	EdgeList.push_back(Line1);
	if (Line1)
	{
		Line1->WallValue = nullptr;
		Line1->place_in_grid();
	}

	auto floatArr5WallPoint = loader::query_float_attribute(groupIndex, 0, 1301);
	Wall1PointFirst = 1 << static_cast<int>(floor(floatArr5WallPoint[0]));
	auto wallPt1_2 = static_cast<int>(floor(floatArr5WallPoint[1]));
	Wall1PointLast = floatArr5WallPoint[7];
	maths::find_closest_edge(
		RampPlane,
		RampPlaneCount,
		reinterpret_cast<wall_point_type*>(floatArr5WallPoint + 3),
		&end2,
		&start2);
	Line2 = new TLine(this, &ActiveFlag, CollisionGroup, start2, end2);
	EdgeList.push_back(Line2);
	if (Line2)
	{
		Line2->WallValue = nullptr;
		Line2->place_in_grid();
	}

	auto floatArr6WallPoint = loader::query_float_attribute(groupIndex, 0, 1302);
	auto wall2Pt1_2 = static_cast<int>(floor(floatArr6WallPoint[1]));
	Wall2PointFirst = 1 << static_cast<int>(floor(floatArr6WallPoint[0]));
	Wall2PointLast = floatArr6WallPoint[7];
	maths::find_closest_edge(
		RampPlane,
		RampPlaneCount,
		reinterpret_cast<wall_point_type*>(floatArr6WallPoint + 3),
		&end3,
		&start3);
	Line3 = new TLine(this, &ActiveFlag, CollisionGroup, start3, end3);
	EdgeList.push_back(Line3);
	if (Line3)
	{
		Line3->WallValue = nullptr;
		Line3->place_in_grid();
	}


	auto xMin = 1000000000.0f;
	auto yMin = 1000000000.0f;
	auto yMax = -1000000000.0f;
	auto xMax = -1000000000.0f;
	for (auto index = 0; index < RampPlaneCount; index++)
	{
		auto plane = &RampPlane[index];
		auto pVec1 = reinterpret_cast<vector_type*>(&plane->V1);
		auto pVec2 = reinterpret_cast<vector_type*>(&plane->V2);
		auto pVec3 = reinterpret_cast<vector_type*>(&plane->V3);

		xMin = std::min(std::min(std::min(plane->V3.X, plane->V1.X), plane->V2.X), xMin);
		yMin = std::min(std::min(std::min(plane->V3.Y, plane->V1.Y), plane->V2.Y), xMin); // Sic
		xMax = std::max(std::max(std::max(plane->V3.X, plane->V1.X), plane->V2.X), xMin);
		yMax = std::max(std::max(std::max(plane->V3.Y, plane->V1.Y), plane->V2.Y), xMin);

		vector_type* pointOrder[4] = {pVec1, pVec2, pVec3, pVec1};
		for (auto pt = 0; pt < 3; pt++)
		{
			auto point1 = pointOrder[pt], point2 = pointOrder[pt + 1];
			auto collisionGroup = 0;
			if (point1 != end2 || point2 != start2)
			{
				if (point1 != end3 || point2 != start3)
				{
					collisionGroup = visual.CollisionGroup;
				}
				else if (wall2Pt1_2)
				{
					collisionGroup = Wall2PointFirst;
				}
			}
			else if (wallPt1_2)
			{
				collisionGroup = Wall1PointFirst;
			}
			if (collisionGroup)
			{
				auto line = new TLine(this, &ActiveFlag, collisionGroup, point1, point2);
				EdgeList.push_back(line);
				if (line)
				{
					line->WallValue = plane;
					line->place_in_grid();
				}
			}
		}

		plane->FieldForce.X = cos(plane->GravityAngle2) * sin(plane->GravityAngle1) *
			PinballTable->GravityDirVectMult;
		plane->FieldForce.Y = sin(plane->GravityAngle2) * sin(plane->GravityAngle1) *
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

void TRamp::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
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
			ball->FieldFlag = Wall1PointFirst;
			if (!RampFlag1)
				return;
			ball->Position.Z = ball->Offset + Wall1PointLast;
		}
		else
		{
			ball->FieldFlag = Wall2PointFirst;
			if (!RampFlag1)
				return;
			ball->Position.Z = ball->Offset + Wall2PointLast;
		}
	}
}

int TRamp::FieldEffect(TBall* ball, vector_type* vecDst)
{
	vecDst->X = ball->RampFieldForce.X - ball->Acceleration.X * ball->Speed * BallFieldMult;
	vecDst->Y = ball->RampFieldForce.Y - ball->Acceleration.Y * ball->Speed * BallFieldMult;
	return 1;
}

void TRamp::port_draw()
{
	TCollisionComponent::port_draw();
}
