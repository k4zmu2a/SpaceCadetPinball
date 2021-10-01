#include "pch.h"
#include "nudge.h"


#include "pb.h"
#include "render.h"
#include "TBall.h"
#include "timer.h"
#include "TPinballTable.h"

int nudge::nudged_left;
int nudge::nudged_right;
int nudge::nudged_up;
int nudge::timer;
float nudge::nudge_count;

void nudge::un_nudge_right(int timerId, void* caller)
{
	if (nudged_right)
		_nudge(-2.0, -1.0);
	nudged_right = 0;
}

void nudge::un_nudge_left(int timerId, void* caller)
{
	if (nudged_left)
		_nudge(2.0, -1.0);
	nudged_left = 0;
}

void nudge::un_nudge_up(int timerId, void* caller)
{
	if (nudged_up)
		_nudge(0.0, -1.0);
	nudged_up = 0;
}

void nudge::nudge_right()
{
	_nudge(2.0, 1.0);
	if (timer)
		timer::kill(timer);
	timer = timer::set(0.4f, nullptr, un_nudge_right);
	nudged_right = 1;
}

void nudge::nudge_left()
{
	_nudge(-2.0, 1.0);
	if (timer)
		timer::kill(timer);
	timer = timer::set(0.4f, nullptr, un_nudge_left);
	nudged_left = 1;
}

void nudge::nudge_up()
{
	_nudge(0.0, 1.0);
	if (timer)
		timer::kill(timer);
	timer = timer::set(0.4f, nullptr, un_nudge_up);
	nudged_up = 1;
}

void nudge::_nudge(float xDiff, float yDiff)
{
	vector_type accelMod;
	float invAccelX, invAccelY;

	accelMod.X = xDiff * 0.5f;
	accelMod.Y = yDiff * 0.5f;
	for (auto ball : pb::MainTable->BallList)
	{
		if (ball->ActiveFlag && !ball->CollisionComp)
		{
			ball->Acceleration.X = ball->Acceleration.X * ball->Speed;
			ball->Acceleration.Y = ball->Acceleration.Y * ball->Speed;
			maths::vector_add(&ball->Acceleration, &accelMod);
			ball->Speed = maths::normalize_2d(&ball->Acceleration);
			if (ball->Acceleration.X == 0.0f)
				invAccelX = 1000000000.0;
			else
				invAccelX = 1.0f / ball->Acceleration.X;
			ball->InvAcceleration.X = invAccelX;
			if (ball->Acceleration.Y == 0.0f)
				invAccelY = 1000000000.0;
			else
				invAccelY = 1.0f / ball->Acceleration.Y;
			ball->InvAcceleration.Y = invAccelY;
		}
	}

	render::shift(static_cast<int>(floor(xDiff + 0.5f)), static_cast<int>(floor(0.5f - yDiff)));
}
