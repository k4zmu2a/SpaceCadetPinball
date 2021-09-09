#include "pch.h"
#include "TBall.h"


#include "fullscrn.h"
#include "loader.h"
#include "maths.h"
#include "objlist_class.h"
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
	FieldFlag = 1;
	CollisionFlag = 0;
	Speed = 0.0;
	Acceleration.Y = 0.0;
	Acceleration.X = 0.0;
	InvAcceleration.Y = 1000000000.0;
	InvAcceleration.X = 1000000000.0;
	Position.X = 0.0;
	Position.Y = 0.0;

	ListBitmap = new objlist_class<gdrv_bitmap8>(0, 4);

	/*Full tilt: ball is ballN, where N[0,2] resolution*/
	if (pb::FullTiltMode)
		ballGroupName[4] = '0' + fullscrn::GetResolution();
	auto groupIndex = loader::query_handle(ballGroupName);

	Offset = *loader::query_float_attribute(groupIndex, 0, 500);
	auto visualCount = loader::query_visual_states(groupIndex);
	auto index = 0;
	if (visualCount > 0)
	{
		auto visualZPtr = VisualZArray;
		do
		{
			loader::query_visual(groupIndex, index, &visual);
			if (ListBitmap)
				ListBitmap->Add(visual.Bitmap);
			auto visVec = reinterpret_cast<vector_type*>(loader::query_float_attribute(groupIndex, index, 501));
			auto zDepth = proj::z_distance(visVec);
			++index;
			*visualZPtr = zDepth;
			++visualZPtr;
		}
		while (index < visualCount);
	}
	RenderSprite = render::create_sprite(VisualTypes::Ball, nullptr, nullptr, 0, 0, nullptr);
	PinballTable->CollisionCompOffset = Offset;
	Position.Z = Offset;
}

void TBall::Repaint()
{
	int pos2D[2];

	if (CollisionFlag)
	{
		Position.Z =
			CollisionOffset.X * Position.X +
			CollisionOffset.Y * Position.Y +
			Offset + CollisionOffset.Z;
	}

	proj::xform_to_2d(&Position, pos2D);
	auto zDepth = proj::z_distance(&Position);

	auto zArrPtr = VisualZArray;
	int index;
	for (index = 0; index < ListBitmap->GetCount() - 1; ++index, zArrPtr++)
	{
		if (*zArrPtr <= zDepth) break;
	}

	auto bmp = ListBitmap->Get(index);
	render::ball_set(
		RenderSprite,
		bmp,
		zDepth,
		pos2D[0] - bmp->Width / 2,
		pos2D[1] - bmp->Height / 2);
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
		FieldFlag = 1;
		Acceleration.Y = 0.0;
		Position.Z = Offset;
		Acceleration.X = 0.0;
		Speed = 0.0;
		RayMaxDistance = 0.0;
	}
	return 0;
}

void TBall::throw_ball(TBall* ball, vector_type* acceleration, float angleMult, float speedMult1, float speedMult2)
{
	ball->CollisionComp = nullptr;
	ball->Acceleration = *acceleration;
	float rnd = RandFloat();
	float angle = (1.0f - (rnd + rnd)) * angleMult;
	maths::RotateVector(&ball->Acceleration, angle);
	rnd = RandFloat();
	ball->Speed = (1.0f - (rnd + rnd)) * (speedMult1 * speedMult2) + speedMult1;
}
