#include "pch.h"
#include "TKickout.h"


#include "control.h"
#include "loader.h"
#include "objlist_class.h"
#include "TBall.h"
#include "TCircle.h"
#include "timer.h"
#include "TPinballTable.h"
#include "TTableLayer.h"

TKickout::TKickout(TPinballTable* table, int groupIndex, bool someFlag): TCollisionComponent(
	table, groupIndex, false)
{
	visualStruct visual{};
	circle_type circle{};

	NotSomeFlag = !someFlag;
	if (!someFlag)
		UnknownBaseFlag2 = 0;
	TimerTime1 = 1.5;
	TimerTime2 = 0.05f;
	MessageField = 0;
	Timer = 0;
	KickFlag1 = 0;
	FieldMult = *loader::query_float_attribute(groupIndex, 0, 305);
	loader::query_visual(groupIndex, 0, &visual);
	SoundIndex2 = visual.SoundIndex2;
	SoundIndex1 = visual.Kicker.SoundIndex;

	Circle.Center.X = *visual.FloatArr;
	Circle.Center.Y = visual.FloatArr[1];
	Circle.RadiusSq = *loader::query_float_attribute(groupIndex, 0, 306) * visual.FloatArr[2];
	if (Circle.RadiusSq == 0.0)
		Circle.RadiusSq = 0.001f;
	auto tCircle = new TCircle(this, &UnknownBaseFlag2, visual.Flag,
	                           reinterpret_cast<vector_type*>(visual.FloatArr), Circle.RadiusSq);
	if (tCircle)
	{
		tCircle->place_in_grid();
		EdgeList->Add(tCircle);
	}

	Circle.RadiusSq = visual.FloatArr[2] * visual.FloatArr[2];
	CollisionBallSetZ = loader::query_float_attribute(groupIndex, 0, 408)[2];
	ThrowSpeedMult2 = visual.Kicker.Unknown3F * 0.01f;
	BallAcceleration.X = visual.Kicker.Unknown4F;
	BallAcceleration.Y = visual.Kicker.Unknown5F;
	BallAcceleration.Z = visual.Kicker.Unknown6F;
	ThrowAngleMult = visual.Kicker.Unknown7F;
	ThrowSpeedMult1 = visual.Kicker.Unknown2F;

	circle.RadiusSq = Circle.RadiusSq;
	circle.Center.X = Circle.Center.X;
	circle.Center.Y = Circle.Center.Y;
	circle.Center.Z = Circle.Center.Z;
	Field.Flag2Ptr = &UnknownBaseFlag2;
	Field.CollisionComp = this;
	Field.Mask = visual.Flag;
	TTableLayer::edges_insert_circle(&circle, nullptr, &Field);
}

int TKickout::Message(int code, float value)
{
	switch (code)
	{
	case 55:
		if (KickFlag1)
		{
			if (value < 0.0)
				value = TimerTime1;
			Timer = timer::set(value, this, TimerExpired);
		}
		break;
	case 1011:
		if (NotSomeFlag)
			UnknownBaseFlag2 = 0;
		break;
	case 1024:
		if (KickFlag1)
		{
			if (Timer)
				timer::kill(Timer);
			TimerExpired(0, this);
		}
		if (NotSomeFlag)
			UnknownBaseFlag2 = 0;
		break;
	default:
		break;
	}

	return 0;
}

void TKickout::put_scoring(int index, int score)
{
	if (index < 5)
		Scores[index] = score;
}

int TKickout::get_scoring(int index)
{
	return index < 5 ? Scores[index] : 0;
}

void TKickout::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
{
	if (!KickFlag1)
	{
		Ball = ball;
		MaxCollisionSpeed = 1000000000.0;
		KickFlag1 = 1;
		ball->CollisionComp = this;
		ball->Position.X = Circle.Center.X;
		ball->Position.Y = Circle.Center.Y;		
		ball->Position.Z = CollisionBallSetZ;
		FieldBallZSet = ball->Position.Z;
		if (PinballTable->TiltLockFlag)
		{
			Message(55, 0.1f);
		}
		else
		{
			loader::play_sound(SoundIndex2);
			control::handler(63, this);
		}
	}
}

int TKickout::FieldEffect(TBall* ball, vector_type* dstVec)
{
	vector_type direction{};

	if (KickFlag1)
		return 0;
	direction.X = Circle.Center.X - ball->Position.X;
	direction.Y = Circle.Center.Y - ball->Position.Y;
	if (direction.Y * direction.Y + direction.X * direction.X > Circle.RadiusSq)
		return 0;
	maths::normalize_2d(&direction);
	dstVec->X = direction.X * FieldMult - ball->Acceleration.X * ball->Speed;
	dstVec->Y = direction.Y * FieldMult - ball->Acceleration.Y * ball->Speed;
	return 1;
}

void TKickout::TimerExpired(int timerId, void* caller)
{
	auto kick = static_cast<TKickout*>(caller);
	if (kick->KickFlag1)
	{
		kick->KickFlag1 = 0;
		kick->Timer = timer::set(kick->TimerTime2, kick, ResetTimerExpired);
		if (kick->Ball)
		{		
			kick->Ball->Position.Z = kick->FieldBallZSet;
			TBall::throw_ball(kick->Ball, &kick->BallAcceleration, kick->ThrowAngleMult, kick->ThrowSpeedMult1,
			                  kick->ThrowSpeedMult2);
			kick->UnknownBaseFlag2 = 0;
			kick->Ball = nullptr;
			loader::play_sound(kick->SoundIndex1);
		}
	}
}

void TKickout::ResetTimerExpired(int timerId, void* caller)
{
	auto kick = static_cast<TKickout*>(caller);
	if (!kick->NotSomeFlag)
		kick->UnknownBaseFlag2 = 1;
	kick->Timer = 0;
}
