#include "pch.h"
#include "THole.h"


#include "control.h"
#include "loader.h"
#include "pb.h"
#include "TBall.h"
#include "timer.h"
#include "TPinballTable.h"
#include "TTableLayer.h"

THole::THole(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
{
	visualStruct visual{};
	circle_type circle{};

	Unknown4 = 0.050000001f;
	MessageField = 0;
	Timer = 0;
	BallCapturedFlag = 0;
	Unknown3 = loader::query_float_attribute(groupIndex, 0, 407, 0.25f);
	GravityMult = loader::query_float_attribute(groupIndex, 0, 701, 0.2f);
	GravityPull = *loader::query_float_attribute(groupIndex, 0, 305);

	loader::query_visual(groupIndex, 0, &visual);
	Circle.Center.X = visual.FloatArr[0];
	Circle.Center.Y = visual.FloatArr[1];
	Circle.RadiusSq = *loader::query_float_attribute(groupIndex, 0, 306) * visual.FloatArr[2];
	if (Circle.RadiusSq == 0.0f)
		Circle.RadiusSq = 0.001f;

	auto tCircle = new TCircle(this, &ActiveFlag, visual.CollisionGroup,
	                           reinterpret_cast<vector_type*>(visual.FloatArr),
	                           Circle.RadiusSq);
	if (tCircle)
	{
		tCircle->place_in_grid();
		EdgeList.push_back(tCircle);
	}

	ZSetValue = loader::query_float_attribute(groupIndex, 0, 408)[2];
	FieldFlag = static_cast<int>(floor(*loader::query_float_attribute(groupIndex, 0, 1304)));

	/*Full tilt hack - FieldFlag should be on*/
	if (pb::FullTiltMode)
		FieldFlag = 1;

	Circle.RadiusSq = visual.FloatArr[2] * visual.FloatArr[2];
	circle.RadiusSq = Circle.RadiusSq;
	circle.Center.X = Circle.Center.X;
	circle.Center.Y = Circle.Center.Y;
	circle.Center.Z = Circle.Center.Z;

	Field.Flag2Ptr = &ActiveFlag;
	Field.CollisionComp = this;
	Field.Mask = visual.CollisionGroup;
	TTableLayer::edges_insert_circle(&circle, nullptr, &Field);
}

int THole::Message(int code, float value)
{
	if (code == 1024 && BallCapturedFlag)
	{
		if (Timer)
			timer::kill(Timer);
		Timer = 0;
		BallCapturedSecondStage = 1;
	}
	return 0;
}

void THole::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
{
	if (!BallCapturedFlag)
	{
		BallCapturedSecondStage = 0;
		Threshold = 1000000000.0;
		BallCapturedFlag = 1;
		ball->CollisionComp = this;
		ball->Position.X = Circle.Center.X;
		ball->Position.Y = Circle.Center.Y;
		ball->Acceleration.Z = 0.0;

		// Ramp hole has no delay in FT.
		auto captureTime = pb::FullTiltMode ? 0 : 0.5f;
		Timer = timer::set(captureTime, this, TimerExpired);

		if (!PinballTable->TiltLockFlag)
		{
			loader::play_sound(HardHitSoundId);
			control::handler(57, this);
		}
	}
}

int THole::FieldEffect(TBall* ball, vector_type* vecDst)
{
	int result;
	vector_type direction{};

	if (BallCapturedFlag)
	{
		if (BallCapturedSecondStage)
		{
			ball->Acceleration.Z -= PinballTable->GravityDirVectMult * ball->TimeDelta * GravityMult;
			ball->Position.Z += ball->Acceleration.Z;
			if (ball->Position.Z <= ZSetValue)
			{
				BallCapturedFlag = 0;
				BallCapturedSecondStage = 0;
				ball->Position.Z = ZSetValue;
				ball->Acceleration.Z = 0.0;
				ball->FieldFlag = FieldFlag;
				ball->Acceleration.Y = 0.0;
				ball->CollisionComp = nullptr;
				ball->Acceleration.X = 0.0;
				ball->Speed = 0.0;
				loader::play_sound(SoftHitSoundId);
				control::handler(58, this);
			}
		}
		result = 0;
	}
	else
	{
		direction.X = Circle.Center.X - ball->Position.X;
		direction.Y = Circle.Center.Y - ball->Position.Y;
		if (direction.X * direction.X + direction.Y * direction.Y <= Circle.RadiusSq)
		{
			maths::normalize_2d(&direction);
			vecDst->X = direction.X * GravityPull - ball->Acceleration.X * ball->Speed;
			vecDst->Y = direction.Y * GravityPull - ball->Acceleration.Y * ball->Speed;
			result = 1;
		}
		else
		{
			result = 0;
		}
	}
	return result;
}

void THole::TimerExpired(int timerId, void* caller)
{
	auto hole = static_cast<THole*>(caller);
	hole->Timer = 0;
	hole->BallCapturedSecondStage = 1;
}
