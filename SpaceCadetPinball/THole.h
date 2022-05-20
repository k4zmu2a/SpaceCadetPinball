#pragma once
#include "TCircle.h"
#include "TCollisionComponent.h"
#include "TEdgeManager.h"

class THole :
	public TCollisionComponent
{
public:
	THole(TPinballTable* table, int groupIndex);
	int Message(int code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
	               TEdgeSegment* edge) override;
	int FieldEffect(TBall* ball, vector2* vecDst) override;

	static void TimerExpired(int timerId, void* caller);

	int BallCapturedFlag;
	int BallCapturedSecondStage{};
	int Timer;
	float Unknown3;
	float Unknown4;
	float GravityMult;
	float ZSetValue;
	int CollisionMask;
	float GravityPull;
	circle_type Circle{};
	field_effect_type Field{};
};
