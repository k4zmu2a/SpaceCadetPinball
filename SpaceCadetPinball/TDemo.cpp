#include "pch.h"
#include "TDemo.h"


#include "loader.h"
#include "pb.h"
#include "TEdgeSegment.h"
#include "timer.h"
#include "TPinballTable.h"
#include "TBall.h"

TDemo::TDemo(TPinballTable* table, int groupIndex)
	: TCollisionComponent(table, groupIndex, false)
{
	visualStruct visual{};

	RestartGameTimer = 0;
	PlungerFlag = 0;
	FlipLeftTimer = 0;
	FlipRightTimer = 0;
	MessageField = 0;
	UnusedBaseFlag = 0;
	ActiveFlag = 0;
	FlipRightFlag = 0;
	FlipLeftFlag = 0;
	table->Demo = this;
	auto floatArr1 = loader::query_float_attribute(groupIndex, 0, 407);
	if (floatArr1)
	{
		FlipTimerTime1 = floatArr1[0];
		FlipTimerTime2 = floatArr1[1];
		UnFlipTimerTime1 = floatArr1[2];
		UnFlipTimerTime2 = floatArr1[3];
	}
	else
	{
		FlipTimerTime1 = 0.2f;
		FlipTimerTime2 = 0.1f;
		UnFlipTimerTime1 = 0.2f;
		UnFlipTimerTime2 = 0.1f;
	}
	loader::query_visual(groupIndex, 0, &visual);
	auto v5 = loader::query_float_attribute(groupIndex, 0, 1400);
	Edge1 = TEdgeSegment::install_wall(v5, this, &ActiveFlag, visual.CollisionGroup, 0.0, 1400);

	auto v6 = loader::query_float_attribute(groupIndex, 0, 1401);
	TEdgeSegment::install_wall(v6, this, &ActiveFlag, visual.CollisionGroup, 0.0, 1401);

	auto v7 = loader::query_float_attribute(groupIndex, 0, 1402);
	Edge2 = TEdgeSegment::install_wall(v7, this, &ActiveFlag, visual.CollisionGroup, 0.0, 1402);

	auto v8 = loader::query_float_attribute(groupIndex, 0, 1403);
	TEdgeSegment::install_wall(v8, this, &ActiveFlag, visual.CollisionGroup, 0.0, 1403);

	auto v9 = loader::query_float_attribute(groupIndex, 0, 1404);
	Edge3 = TEdgeSegment::install_wall(v9, this, &ActiveFlag, visual.CollisionGroup, table->CollisionCompOffset, 1404);
}

int TDemo::Message(int code, float value)
{
	switch (code)
	{
	case 1014:
		if (RestartGameTimer)
			timer::kill(RestartGameTimer);
		RestartGameTimer = 0;
		break;
	case 1022:
		if (RestartGameTimer)
			timer::kill(RestartGameTimer);
		RestartGameTimer = 0;
		if (ActiveFlag != 0)
			RestartGameTimer = timer::set(5.0, this, NewGameRestartTimer);
		break;
	case 1024:
		if (FlipLeftTimer)
			timer::kill(FlipLeftTimer);
		FlipLeftTimer = 0;
		if (FlipRightTimer)
			timer::kill(FlipRightTimer);
		FlipRightTimer = 0;

		if (FlipLeftFlag != 0)
			UnFlipLeft(0, this);
		if (FlipRightFlag)
			UnFlipRight(0, this);
		if (PlungerFlag)
			PlungerRelease(0, this);
		break;
	default:
		break;
	}
	return 0;
}

void TDemo::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
{
	ball->not_again(edge);
	ball->Position.X = nextPosition->X;
	ball->Position.Y = nextPosition->Y;
	ball->RayMaxDistance -= coef;

	switch (reinterpret_cast<size_t>(edge->WallValue))
	{
	case 1400:
		if (!FlipLeftTimer && !FlipLeftFlag)
		{
			float time = FlipTimerTime1 + FlipTimerTime2 - RandFloat() * (FlipTimerTime2 + FlipTimerTime2);
			FlipLeftTimer = timer::set(time, this, FlipLeft);
		}
		break;
	case 1401:
		FlipLeft(0, this);
		break;
	case 1402:
		if (!FlipRightTimer && !FlipRightFlag)
		{
			float time = FlipTimerTime1 + FlipTimerTime2 - RandFloat() * (FlipTimerTime2 + FlipTimerTime2);
			FlipRightTimer = timer::set(time, this, FlipRight);
		}
		break;
	case 1403:
		FlipRight(0, this);
		break;
	case 1404:
		if (!PlungerFlag)
		{
			PinballTable->Message(1004, ball->TimeNow);
			float time = RandFloat() + 2.0f;
			PlungerFlag = timer::set(time, this, PlungerRelease);
		}
		break;
	default:
		break;
	}
}

void TDemo::PlungerRelease(int timerId, void* caller)
{
	auto demo = static_cast<TDemo*>(caller);
	demo->PlungerFlag = 0;
	demo->PinballTable->Message(1005, pb::time_next);
}

void TDemo::UnFlipRight(int timerId, void* caller)
{
	auto demo = static_cast<TDemo*>(caller);
	if (demo->FlipRightFlag)
		demo->PinballTable->Message(1003, pb::time_next);
	demo->FlipRightFlag = 0;
}

void TDemo::UnFlipLeft(int timerId, void* caller)
{
	auto demo = static_cast<TDemo*>(caller);
	if (demo->FlipLeftFlag)
		demo->PinballTable->Message(1001, pb::time_next);
	demo->FlipLeftFlag = 0;
}

void TDemo::FlipRight(int timerId, void* caller)
{
	auto demo = static_cast<TDemo*>(caller);
	if (!demo->FlipRightFlag)
	{
		if (demo->FlipRightTimer)
		{
			timer::kill(demo->FlipRightTimer);
			demo->FlipRightTimer = 0;
		}
		demo->PinballTable->Message(1002, pb::time_next);
		demo->FlipRightFlag = 1;
		float time = demo->UnFlipTimerTime1 + demo->UnFlipTimerTime2 - RandFloat() *
			(demo->UnFlipTimerTime2 + demo->UnFlipTimerTime2);
		timer::set(time, demo, UnFlipRight);
	}
}

void TDemo::FlipLeft(int timerId, void* caller)
{
	auto demo = static_cast<TDemo*>(caller);
	if (!demo->FlipLeftFlag)
	{
		if (demo->FlipLeftTimer)
		{
			timer::kill(demo->FlipLeftTimer);
			demo->FlipLeftTimer = 0;
		}
		demo->PinballTable->Message(1000, pb::time_next);
		demo->FlipLeftFlag = 1;
		float time = demo->UnFlipTimerTime1 + demo->UnFlipTimerTime2 - RandFloat() *
			(demo->UnFlipTimerTime2 + demo->UnFlipTimerTime2);
		timer::set(time, demo, UnFlipLeft);
	}
}

void TDemo::NewGameRestartTimer(int timerId, void* caller)
{
	auto demo = static_cast<TDemo*>(caller);
	pb::replay_level(1);
	demo->PinballTable->Message(1014, static_cast<float>(demo->PinballTable->PlayerCount));
	demo->RestartGameTimer = 0;
}
