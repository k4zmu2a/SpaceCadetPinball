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
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
	               TEdgeSegment* edge) override;
	int FieldEffect(TBall* ball, vector_type* vecDst) override;

	static void TimerExpired(int timerId, void* caller);

	int BallCapturedFlag;
	int BallCapturedSecondStage{};
	int Timer;
	float Unknown3;
	float Unknown4;
	float GravityMult;
	float ZSetValue;
	int FieldFlag;
	float GravityPull;
	circle_type Circle{};
	field_effect_type Field{};
};
