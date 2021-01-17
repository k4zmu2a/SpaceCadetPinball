#pragma once
#include "TCollisionComponent.h"

class TDrain :
	public TCollisionComponent
{
public:
	TDrain(TPinballTable* table, int groupIndex);
	int Message(int code, float value) override;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
	               TEdgeSegment* edge) override;

	static void TimerCallback(int timerId, void* caller);

	float TimerTime;
	int Timer;
};
