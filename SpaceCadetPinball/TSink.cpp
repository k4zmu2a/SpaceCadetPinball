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
	loader::query_visual(groupIndex, 0, &visual);
	BallThrowDirection = visual.Kicker.ThrowBallDirection;
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

int TSink::Message(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::TSinkResetTimer:
		if (value < 0.0f)
			value = TimerTime;
		timer::set(value, this, TimerExpired);
		break;
	case MessageCode::PlayerChanged:
		timer::kill(TimerExpired);
		PlayerMessagefieldBackup[PinballTable->CurrentPlayer] = MessageField;
		MessageField = PlayerMessagefieldBackup[static_cast<int>(floor(value))];
		break;
	case MessageCode::Reset:
		{
			timer::kill(TimerExpired);
			MessageField = 0;
			for (auto &msgBackup : PlayerMessagefieldBackup)
				msgBackup = 0;
			break;
		}
	default:
		break;
	}
	return 0;
}

void TSink::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance, TEdgeSegment* edge)
{
	if (PinballTable->TiltLockFlag)
	{
		maths::basic_collision(ball, nextPosition, direction, Elasticity, Smoothness, 1000000000.0, 0.0);
	}
	else
	{
		ball->Disable();
		loader::play_sound(SoundIndex4, ball, "TSink1");
		control::handler(MessageCode::ControlCollision, this);
	}
}

void TSink::TimerExpired(int timerId, void* caller)
{
	RectF rect{};

	auto sink = static_cast<TSink*>(caller);
	auto table = sink->PinballTable;

	rect.XMin = table->CollisionCompOffset * -2.0f + sink->BallPosition.X;
	rect.XMax = table->CollisionCompOffset * 2.0f + sink->BallPosition.X;
	rect.YMin = table->CollisionCompOffset * -2.0f + sink->BallPosition.Y;
	rect.YMax = table->CollisionCompOffset * 2.0f + sink->BallPosition.Y;
	if (table->BallCountInRect(rect))
	{
		timer::set(0.5f, sink, TimerExpired);
	}
	else 
	{
		auto ball = table->AddBall(sink->BallPosition.X, sink->BallPosition.Y);
		assertm(ball, "Failure to create ball in sink");
		ball->AsEdgeCollisionFlag = true;
		ball->throw_ball(&sink->BallThrowDirection, sink->ThrowAngleMult, sink->ThrowSpeedMult1,
			sink->ThrowSpeedMult2);
		if (sink->SoundIndex3)
			loader::play_sound(sink->SoundIndex3, ball, "TSink2");
	}
}
