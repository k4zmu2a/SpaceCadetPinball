#include "pch.h"
#include "TLightRollover.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "TBall.h"
#include "timer.h"
#include "TPinballTable.h"

TLightRollover::TLightRollover(TPinballTable* table, int groupIndex) : TRollover(table, groupIndex, false)
{
	RolloverFlag = 0;
	Timer = 0;
	if (ListBitmap != nullptr)
		render::sprite_set_bitmap(RenderSprite, nullptr);
	build_walls(groupIndex);
	FloatArr = *loader::query_float_attribute(groupIndex, 0, 407);
}

int TLightRollover::Message(int code, float value)
{
	if (code == 1024)
	{
		ActiveFlag = 1;
		RolloverFlag = 0;
		if (Timer)
			timer::kill(Timer);
		Timer = 0;
		if (ListBitmap)
			render::sprite_set_bitmap(RenderSprite, nullptr);
	}
	return 0;
}

void TLightRollover::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
                               TEdgeSegment* edge)
{
	ball->Position.X = nextPosition->X;
	ball->Position.Y = nextPosition->Y;
	ball->RayMaxDistance -= coef;
	ball->not_again(edge);
	if (!PinballTable->TiltLockFlag)
	{
		if (RolloverFlag)
		{
			timer::set(0.1f, this, TimerExpired);
			ActiveFlag = 0;
			RolloverFlag = RolloverFlag == 0;
			if (Timer == 0)
				Timer = timer::set(FloatArr, this, delay_expired);
		}
		else
		{
			loader::play_sound(SoftHitSoundId);
			control::handler(63, this);
			RolloverFlag = RolloverFlag == 0;
			if (ListBitmap)
				render::sprite_set_bitmap(RenderSprite, ListBitmap->at(0));
		}
	}
}

void TLightRollover::delay_expired(int timerId, void* caller)
{
	auto roll = static_cast<TLightRollover*>(caller);
	render::sprite_set_bitmap(roll->RenderSprite, nullptr);
	roll->Timer = 0;
}
