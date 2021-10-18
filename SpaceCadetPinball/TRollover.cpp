#include "pch.h"
#include "TRollover.h"


#include "control.h"
#include "gdrv.h"
#include "loader.h"
#include "render.h"
#include "TBall.h"
#include "TEdgeSegment.h"
#include "timer.h"
#include "TPinballTable.h"

TRollover::TRollover(TPinballTable* table, int groupIndex, bool createWall) : TCollisionComponent(
	table, groupIndex, createWall)
{
}


TRollover::TRollover(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
{
	if (ListBitmap)
		render::sprite_set_bitmap(RenderSprite, ListBitmap->at(0));
	build_walls(groupIndex);
}


int TRollover::Message(int code, float value)
{
	if (code == 1024)
	{
		this->ActiveFlag = 1;
		this->RolloverFlag = 0;
		if (this->ListBitmap)
			render::sprite_set_bitmap(this->RenderSprite, this->ListBitmap->at(0));
	}
	return 0;
}

void TRollover::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
                          TEdgeSegment* edge)
{
	ball->Position.X = nextPosition->X;
	ball->Position.Y = nextPosition->Y;
	ball->RayMaxDistance -= coef;
	ball->not_again(edge);
	gdrv_bitmap8* bmp = nullptr;
	if (!PinballTable->TiltLockFlag)
	{
		if (RolloverFlag)
		{
			timer::set(0.1f, this, TimerExpired);
			ActiveFlag = 0;
		}
		else
		{
			loader::play_sound(SoftHitSoundId);
			control::handler(63, this);
		}
		RolloverFlag = RolloverFlag == 0;
		if (ListBitmap)
		{
			if (!RolloverFlag)
				bmp = ListBitmap->at(0);
			render::sprite_set_bitmap(RenderSprite, bmp);
		}
	}
}

void TRollover::put_scoring(int index, int score)
{
	if (index < 2)
		Scores[index] = score;
}

int TRollover::get_scoring(int index)
{
	return index < 2 ? Scores[index] : 0;
}


void TRollover::build_walls(int groupIndex)
{
	visualStruct visual{};

	loader::query_visual(groupIndex, 0, &visual);
	float* arr1 = loader::query_float_attribute(groupIndex, 0, 600);
	TEdgeSegment::install_wall(arr1, this, &ActiveFlag, visual.CollisionGroup, 0.0, 600);
	float* arr2 = loader::query_float_attribute(groupIndex, 0, 603);
	TEdgeSegment::install_wall(arr2, this, &RolloverFlag, visual.CollisionGroup, 0.0, 603);
}

void TRollover::TimerExpired(int timerId, void* caller)
{
	auto roll = static_cast<TRollover*>(caller);
	roll->ActiveFlag = 1;
}
