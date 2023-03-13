#include "pch.h"
#include "TKickout.h"


#include "control.h"
#include "loader.h"
#include "pb.h"
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
		ActiveFlag = 0;
	TimerTime1 = 1.5;
	TimerTime2 = 0.05f;
	MessageField = 0;
	Timer = 0;
	BallCaputeredFlag = 0;
	FieldMult = *loader::query_float_attribute(groupIndex, 0, 305);
	loader::query_visual(groupIndex, 0, &visual);
	SoftHitSoundId = visual.SoftHitSoundId;
	HardHitSoundId = visual.Kicker.HardHitSoundId;

	Circle.Center.X = visual.FloatArr[0];
	Circle.Center.Y = visual.FloatArr[1];
	Circle.RadiusSq = *loader::query_float_attribute(groupIndex, 0, 306) * visual.FloatArr[2];
	if (Circle.RadiusSq == 0.0f)
		Circle.RadiusSq = 0.001f;
	auto tCircle = new TCircle(this, &ActiveFlag, visual.CollisionGroup,
	                           reinterpret_cast<vector3*>(visual.FloatArr), Circle.RadiusSq);
	if (tCircle)
	{
		tCircle->place_in_grid(&AABB);
		EdgeList.push_back(tCircle);
	}

	Circle.RadiusSq = visual.FloatArr[2] * visual.FloatArr[2];
	auto zAttr = loader::query_float_attribute(groupIndex, 0, 408);
	CollisionBallSetZ = pb::FullTiltMode && !pb::FullTiltDemoMode ? zAttr[3] : zAttr[2];
	ThrowSpeedMult2 = visual.Kicker.ThrowBallMult * 0.01f;
	BallThrowDirection = visual.Kicker.ThrowBallDirection;
	ThrowAngleMult = visual.Kicker.ThrowBallAngleMult;
	ThrowSpeedMult1 = visual.Kicker.Boost;

	circle.RadiusSq = Circle.RadiusSq;
	circle.Center.X = Circle.Center.X;
	circle.Center.Y = Circle.Center.Y;
	Field.ActiveFlag = &ActiveFlag;
	Field.CollisionComp = this;
	Field.CollisionGroup = visual.CollisionGroup;
	TTableLayer::edges_insert_circle(&circle, nullptr, &Field);
}

int TKickout::Message(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::TKickoutRestartTimer:
		if (BallCaputeredFlag)
		{
			if (value < 0.0f)
				value = TimerTime1;
			Timer = timer::set(value, this, TimerExpired);
		}
		break;
	case MessageCode::SetTiltLock:
		if (NotSomeFlag)
			ActiveFlag = 0;
		break;
	case MessageCode::Reset:
		if (BallCaputeredFlag)
		{
			if (Timer)
				timer::kill(Timer);
			TimerExpired(0, this);
		}
		if (NotSomeFlag)
			ActiveFlag = 0;
		break;
	default:
		break;
	}

	return 0;
}

void TKickout::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance, TEdgeSegment* edge)
{
	if (!BallCaputeredFlag)
	{
		Ball = ball;
		Threshold = 1000000000.0;
		BallCaputeredFlag = 1;
		ball->CollisionComp = this;
		ball->Position.X = Circle.Center.X;
		ball->Position.Y = Circle.Center.Y;
		OriginalBallZ = ball->Position.Z;
		ball->Position.Z = CollisionBallSetZ;
		ball->CollisionDisabledFlag = true;
		if (PinballTable->TiltLockFlag)
		{
			Message(MessageCode::TKickoutRestartTimer, 0.1f);
		}
		else
		{
			loader::play_sound(SoftHitSoundId, ball, "TKickout1");
			control::handler(MessageCode::ControlCollision, this);
		}
	}
	else
	{
		ball->Position.X = nextPosition->X;
		ball->Position.Y = nextPosition->Y;
		ball->RayMaxDistance -= distance;
		ball->not_again(edge);
	}
}

int TKickout::FieldEffect(TBall* ball, vector2* dstVec)
{
	vector2 direction{};

	if (BallCaputeredFlag)
		return 0;
	direction.X = Circle.Center.X - ball->Position.X;
	direction.Y = Circle.Center.Y - ball->Position.Y;
	if (direction.Y * direction.Y + direction.X * direction.X > Circle.RadiusSq)
		return 0;
	maths::normalize_2d(direction);
	dstVec->X = direction.X * FieldMult - ball->Direction.X * ball->Speed;
	dstVec->Y = direction.Y * FieldMult - ball->Direction.Y * ball->Speed;
	return 1;
}

void TKickout::TimerExpired(int timerId, void* caller)
{
	auto kick = static_cast<TKickout*>(caller);
	if (kick->BallCaputeredFlag)
	{
		kick->BallCaputeredFlag = 0;
		kick->Timer = timer::set(kick->TimerTime2, kick, ResetTimerExpired);
		if (kick->Ball)
		{		
			loader::play_sound(kick->HardHitSoundId, kick->Ball, "TKickout2");
			kick->Ball->Position.Z = kick->OriginalBallZ;
			kick->Ball->throw_ball(&kick->BallThrowDirection, kick->ThrowAngleMult, kick->ThrowSpeedMult1,
			                  kick->ThrowSpeedMult2);
			kick->ActiveFlag = 0;
			kick->Ball = nullptr;
		}
	}
}

void TKickout::ResetTimerExpired(int timerId, void* caller)
{
	auto kick = static_cast<TKickout*>(caller);
	if (!kick->NotSomeFlag)
		kick->ActiveFlag = 1;
	kick->Timer = 0;
}
