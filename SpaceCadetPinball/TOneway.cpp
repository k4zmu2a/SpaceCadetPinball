#include "pch.h"
#include "TOneway.h"


#include "control.h"
#include "loader.h"
#include "TBall.h"
#include "TLine.h"
#include "TPinballTable.h"

TOneway::TOneway(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
{
	visualStruct visual{};
	vector2 linePt1{}, linePt2{};

	loader::query_visual(groupIndex, 0, &visual);
	if (visual.FloatArrCount == 2)
	{
		linePt2.X = visual.FloatArr[0];
		linePt2.Y = visual.FloatArr[1];
		linePt1.X = visual.FloatArr[2];
		linePt1.Y = visual.FloatArr[3];

		auto line = new TLine(this, &ActiveFlag, visual.CollisionGroup, linePt2, linePt1);
		if (line)
		{
			line->Offset(table->CollisionCompOffset);
			line->place_in_grid(&AABB);
			EdgeList.push_back(line);
		}

		line = new TLine(this, &ActiveFlag, visual.CollisionGroup, linePt1, linePt2);
		Line = line;
		if (line)
		{
			line->Offset(-table->CollisionCompOffset * 0.8f);
			Line->place_in_grid(&AABB);
			EdgeList.push_back(Line);
		}
	}
}

void TOneway::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance, TEdgeSegment* edge)
{
	if (edge == Line)
	{
		ball->not_again(edge);
		ball->Position.X = nextPosition->X;
		ball->Position.Y = nextPosition->Y;
		ball->RayMaxDistance -= distance;
		if (!PinballTable->TiltLockFlag)
		{
			if (HardHitSoundId)
				loader::play_sound(HardHitSoundId, ball, "TOneway1");
			control::handler(MessageCode::ControlCollision, this);
		}
	}
	else if (PinballTable->TiltLockFlag)
	{
		maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, 1000000000.0, 0.0);
	}
	else if (maths::basic_collision(
		ball,
		nextPosition,
		direction,
		Elasticity,
		Smoothness,
		Threshold,
		Boost) > 0.2f)
	{
		if (SoftHitSoundId)
			loader::play_sound(SoftHitSoundId, ball, "TOneway2");
	}
}
