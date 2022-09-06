#include "pch.h"
#include "TDrain.h"


#include "control.h"
#include "loader.h"
#include "TBall.h"
#include "timer.h"
#include "TPinballTable.h"

TDrain::TDrain(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	Timer = 0;
	TimerTime = *loader::query_float_attribute(groupIndex, 0, 407);
}

int TDrain::Message(MessageCode code, float value)
{
	if (code == MessageCode::Reset)
	{
		if (Timer)
		{
			timer::kill(Timer);
			Timer = 0;
		}
		PinballTable->BallInDrainFlag = 0;
	}
	return 0;
}

void TDrain::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance, TEdgeSegment* edge)
{
	ball->Disable();
	--PinballTable->MultiballCount;
	if (PinballTable->MultiballCount <= 0)
	{
		PinballTable->MultiballCount = 0;
		PinballTable->BallInDrainFlag = 1;
		Timer = timer::set(TimerTime, this, TimerCallback);
	}
	control::handler(63, this);
}

void TDrain::TimerCallback(int timerId, void* caller)
{
	auto drain = static_cast<TDrain*>(caller);
	control::handler(60, drain);
}
