#include "pch.h"
#include "TWall.h"


#include "control.h"
#include "render.h"
#include "timer.h"

TWall::TWall(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	if (RenderSprite)
		SpriteSet(-1);
}

int TWall::Message(MessageCode code, float value)
{
	if (code == MessageCode::Reset && Timer)
	{
		timer::kill(Timer);
		TimerExpired(Timer, this);
	}
	return 0;
}

void TWall::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance, TEdgeSegment* edge)
{
	if (DefaultCollision(ball, nextPosition, direction))
	{
		if (ListBitmap)
		{
			SpriteSet(0);
			Timer = timer::set(0.1f, this, TimerExpired);
		}
		control::handler(MessageCode::ControlCollision, this);
	}
}

void TWall::TimerExpired(int timerId, void* caller)
{
	auto wall = static_cast<TWall*>(caller);
	wall->SpriteSet(-1);
	wall->Timer = 0;
	wall->MessageField = 0;
}
