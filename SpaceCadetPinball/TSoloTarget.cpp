#include "pch.h"
#include "TSoloTarget.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "timer.h"
#include "TPinballTable.h"

TSoloTarget::TSoloTarget(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	visualStruct visual{};

	Timer = 0;
	TimerTime = 0.1f;
	loader::query_visual(groupIndex, 0, &visual);
	SoundIndex4 = visual.SoundIndex4;
	TSoloTarget::Message(MessageCode::TSoloTargetEnable, 0.0);
}

int TSoloTarget::Message(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::TSoloTargetDisable:
	case MessageCode::TSoloTargetEnable:
		ActiveFlag = code == MessageCode::TSoloTargetEnable;
		break;
	case MessageCode::Reset:
		if (Timer)
			timer::kill(Timer);
		Timer = 0;
		ActiveFlag = 1;
		break;
	default:
		return 0;
	}

	SpriteSet(1 - ActiveFlag);
	return 0;
}

void TSoloTarget::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
                            TEdgeSegment* edge)
{
	if (DefaultCollision(ball, nextPosition, direction))
	{
		Message(MessageCode::TSoloTargetDisable, 0.0);
		Timer = timer::set(TimerTime, this, TimerExpired);
		control::handler(MessageCode::ControlCollision, this);
	}
}

void TSoloTarget::TimerExpired(int timerId, void* caller)
{
	auto target = static_cast<TSoloTarget*>(caller);
	target->Message(MessageCode::TSoloTargetEnable, 0.0);
	target->Timer = 0;
}
