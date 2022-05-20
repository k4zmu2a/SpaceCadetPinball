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
	vector2 accelMod;

	accelMod.X = xDiff * 0.5f;
	accelMod.Y = yDiff * 0.5f;
	for (auto ball : pb::MainTable->BallList)
	{
		if (ball->ActiveFlag && !ball->CollisionComp)
		{
			ball->Direction.X = ball->Direction.X * ball->Speed;
			ball->Direction.Y = ball->Direction.Y * ball->Speed;
			maths::vector_add(ball->Direction, accelMod);
			ball->Speed = maths::normalize_2d(ball->Direction);
		}
	}

	render::shift(static_cast<int>(floor(xDiff + 0.5f)), static_cast<int>(floor(0.5f - yDiff)));
}
