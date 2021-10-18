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
	vector_type linePt1{}, linePt2{};

	loader::query_visual(groupIndex, 0, &visual);
	if (visual.FloatArrCount == 2)
	{
		linePt2.X = visual.FloatArr[0];
		linePt2.Y = visual.FloatArr[1];
		linePt1.X = visual.FloatArr[2];
		linePt1.Y = visual.FloatArr[3];

		auto line = new TLine(this, &ActiveFlag, visual.CollisionGroup, &linePt2, &linePt1);
		if (line)
		{
			line->Offset(table->CollisionCompOffset);
			line->place_in_grid();
			EdgeList.push_back(line);
		}

		line = new TLine(this, &ActiveFlag, visual.CollisionGroup, &linePt1, &linePt2);
		Line = line;
		if (line)
		{
			line->Offset(-table->CollisionCompOffset * 0.8f);
			Line->place_in_grid();
			EdgeList.push_back(Line);
		}
	}
}

void TOneway::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
{
	if (edge == Line)
	{
		ball->not_again(edge);
		ball->Position.X = nextPosition->X;
		ball->Position.Y = nextPosition->Y;
		ball->RayMaxDistance -= coef;
		if (!PinballTable->TiltLockFlag)
		{
			if (HardHitSoundId)
				loader::play_sound(HardHitSoundId);
			control::handler(63, this);
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
			loader::play_sound(SoftHitSoundId);
	}
}

void TOneway::put_scoring(int index, int score)
{
	if (index < 6)
		Scores[index] = score;
}

int TOneway::get_scoring(int index)
{
	return index < 6 ? Scores[index] : 0;
}
