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
	SpriteSet(-1);
	build_walls(groupIndex);
	FloatArr = *loader::query_float_attribute(groupIndex, 0, 407);
}

int TLightRollover::Message(MessageCode code, float value)
{
	if (code == MessageCode::Reset)
	{
		ActiveFlag = 1;
		RolloverFlag = 0;
		if (Timer)
			timer::kill(Timer);
		Timer = 0;
		SpriteSet(-1);
	}
	return 0;
}

void TLightRollover::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
                               TEdgeSegment* edge)
{
	ball->Position.X = nextPosition->X;
	ball->Position.Y = nextPosition->Y;
	ball->RayMaxDistance -= distance;
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
			loader::play_sound(SoftHitSoundId, this, "TLightRollover");
			control::handler(MessageCode::ControlCollision, this);
			RolloverFlag = RolloverFlag == 0;
			SpriteSet(0);
		}
	}
}

void TLightRollover::delay_expired(int timerId, void* caller)
{
	auto roll = static_cast<TLightRollover*>(caller);
	roll->SpriteSet(-1);
	roll->Timer = 0;
}
