#include "pch.h"
#include "TBall.h"


#include "fullscrn.h"
#include "loader.h"
#include "maths.h"
#include "pb.h"
#include "proj.h"
#include "render.h"
#include "TPinballTable.h"

TBall::TBall(TPinballTable* table) : TPinballComponent(table, -1, false)
{
	visualStruct visual{};
	char ballGroupName[10]{"ball"};

	TimeNow = 0.0;
	RayMaxDistance = 0.0;
	ActiveFlag = 1;
	CollisionComp = nullptr;
	EdgeCollisionCount = 0;
	TimeDelta = 0.0;
	CollisionMask = 1;
	CollisionFlag = 0;
	Speed = 0.0;
	Direction.Y = 0.0;
	Direction.X = 0.0;
	Position.X = 0.0;
	Position.Y = 0.0;

	ListBitmap = new std::vector<gdrv_bitmap8*>();

	/*Full tilt: ball is ballN, where N[0,2] resolution*/
	auto groupIndex = loader::query_handle(ballGroupName);
	if (groupIndex < 0)
	{
		ballGroupName[4] = '0' + fullscrn::GetResolution();
		groupIndex = loader::query_handle(ballGroupName);
	}

	Offset = *loader::query_float_attribute(groupIndex, 0, 500);

	auto visualCount = loader::query_visual_states(groupIndex);
	for (auto index = 0; index < visualCount; ++index)
	{
		loader::query_visual(groupIndex, index, &visual);
		if (ListBitmap)
			ListBitmap->push_back(visual.Bitmap);
		auto visVec = reinterpret_cast<vector3*>(loader::query_float_attribute(groupIndex, index, 501));
		auto zDepth = proj::z_distance(*visVec);
		VisualZArray[index] = zDepth;
	}
	RenderSprite = render::create_sprite(VisualTypes::Ball, nullptr, nullptr, 0, 0, nullptr);
	PinballTable->CollisionCompOffset = Offset;
	Position.Z = Offset;
}

void TBall::Repaint()
{
	if (CollisionFlag)
	{
		Position.Z =
			CollisionOffset.X * Position.X +
			CollisionOffset.Y * Position.Y +
			Offset + CollisionOffset.Z;
	}

	auto pos2D = proj::xform_to_2d(Position);
	auto zDepth = proj::z_distance(Position);

	auto zArrPtr = VisualZArray;
	auto index = 0u;
	for (; index < ListBitmap->size() - 1; ++index, zArrPtr++)
	{
		if (*zArrPtr <= zDepth) break;
	}

	auto bmp = ListBitmap->at(index);
	render::ball_set(
		RenderSprite,
		bmp,
		zDepth,
		pos2D.X - bmp->Width / 2,
		pos2D.Y - bmp->Height / 2);
}

void TBall::not_again(TEdgeSegment* edge)
{
	if (EdgeCollisionCount < 5)
	{
		Collisions[EdgeCollisionCount] = edge;
		++EdgeCollisionCount;
	}
}

bool TBall::already_hit(TEdgeSegment* edge)
{
	for (int i = 0; i < EdgeCollisionCount; i++)
	{
		if (Collisions[i] == edge)
			return true;
	}

	return false;
}

int TBall::Message(int code, float value)
{
	if (code == 1024)
	{
		render::ball_set(RenderSprite, nullptr, 0.0, 0, 0);
		Position.X = 0.0;
		CollisionComp = nullptr;
		Position.Y = 0.0;
		ActiveFlag = 0;
		CollisionFlag = 0;
		CollisionMask = 1;
		Direction.Y = 0.0;
		Position.Z = Offset;
		Direction.X = 0.0;
		Speed = 0.0;
		RayMaxDistance = 0.0;
	}
	return 0;
}

void TBall::throw_ball(TBall* ball, vector3* direction, float angleMult, float speedMult1, float speedMult2)
{
	ball->CollisionComp = nullptr;
	ball->Direction = *direction;
	float rnd = RandFloat();
	float angle = (1.0f - (rnd + rnd)) * angleMult;
	maths::RotateVector(ball->Direction, angle);
	rnd = RandFloat();
	ball->Speed = (1.0f - (rnd + rnd)) * (speedMult1 * speedMult2) + speedMult1;
}

vector2 TBall::get_coordinates()
{
	vector2  coordinates;
	vector2i pos2D = proj::xform_to_2d(Position);
	coordinates.X = (float)pos2D.X / PinballTable->Width;
	coordinates.Y = (float)pos2D.Y / PinballTable->Height;
	return coordinates;
}
