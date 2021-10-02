#pragma once
#include "maths.h"
#include "TCollisionComponent.h"
#include "TEdgeManager.h"

class TKickout :
	public TCollisionComponent
{
public:
	TKickout(TPinballTable* table, int groupIndex, bool someFlag);
	int Message(int code, float value) override;
	void put_scoring(int index, int score) override;
	int get_scoring(int index) override;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
	               TEdgeSegment* edge) override;
	int FieldEffect(TBall* ball, vector_type* vecDst) override;

	static void TimerExpired(int timerId, void* caller);
	static void ResetTimerExpired(int timerId, void* caller);

	int KickFlag1;
	int NotSomeFlag;
	int Timer;
	float TimerTime1;
	float TimerTime2;
	float CollisionBallSetZ;
	TBall* Ball{};
	float FieldMult;
	circle_type Circle{};
	float OriginalBallZ{};
	vector_type BallAcceleration{};
	float ThrowAngleMult;
	float ThrowSpeedMult1;
	float ThrowSpeedMult2;
	field_effect_type Field{};
	int Scores[5]{};
};
