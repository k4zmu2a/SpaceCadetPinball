#include "pch.h"
#include "TSink.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "TPinballTable.h"
#include "TBall.h"
#include "timer.h"

TSink::TSink(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	visualStruct visual{};

	MessageField = 0;
	Timer = 0;
	loader::query_visual(groupIndex, 0, &visual);	
	BallAcceleration = visual.Kicker.ThrowBallAcceleration;
	ThrowAngleMult = visual.Kicker.ThrowBallAngleMult;
	ThrowSpeedMult1 = visual.Kicker.Boost;
	ThrowSpeedMult2 = visual.Kicker.ThrowBallMult * 0.01f;
	SoundIndex4 = visual.SoundIndex4;	
	SoundIndex3 = visual.SoundIndex3;
	auto floatArr = loader::query_float_attribute(groupIndex, 0, 601);
	BallPosition.X = floatArr[0];
	BallPosition.Y = floatArr[1];
	TimerTime = *loader::query_float_attribute(groupIndex, 0, 407);
}

int TSink::Message(int code, float value)
{
	switch (code)
	{
	case 56:
		if (value < 0.0f)
			value = TimerTime;
		Timer = timer::set(value, this, TimerExpired);
		break;
	case 1020:
		PlayerMessagefieldBackup[PinballTable->CurrentPlayer] = MessageField;
		MessageField = PlayerMessagefieldBackup[static_cast<int>(floor(value))];
		break;
	case 1024:
		{
			if (Timer)
				timer::kill(Timer);
			Timer = 0;
			MessageField = 0;

			auto playerPtr = PlayerMessagefieldBackup;
			for (auto index = 0; index < PinballTable->PlayerCount; ++index)
			{
				*playerPtr = 0;

				++playerPtr;
			}

			break;
		}
	default:
		break;
	}
	return 0;
}

void TSink::put_scoring(int index, int score)
{
	if (index < 3)
		Scores[index] = score;
}

int TSink::get_scoring(int index)
{
	return index < 3 ? Scores[index] : 0;
}

void TSink::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
{
	Timer = 0;
	if (PinballTable->TiltLockFlag)
	{
		maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, 1000000000.0, 0.0);
	}
	else
	{
		ball->ActiveFlag = 0;
		render::sprite_set_bitmap(ball->RenderSprite, nullptr);
		loader::play_sound(SoundIndex4);
		control::handler(63, this);
	}
}

void TSink::TimerExpired(int timerId, void* caller)
{
	auto sink = static_cast<TSink*>(caller);
	auto ball = sink->PinballTable->BallList.at(0);
	ball->CollisionComp = nullptr;
	ball->ActiveFlag = 1;
	ball->Position.X = sink->BallPosition.X;
	ball->Position.Y = sink->BallPosition.Y;
	TBall::throw_ball(ball, &sink->BallAcceleration, sink->ThrowAngleMult, sink->ThrowSpeedMult1,
	                  sink->ThrowSpeedMult2);
	if (sink->SoundIndex3)
		loader::play_sound(sink->SoundIndex3);
	sink->Timer = 0;
}
