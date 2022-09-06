#pragma once
#include "TCollisionComponent.h"

class TDrain :
	public TCollisionComponent
{
public:
	TDrain(TPinballTable* table, int groupIndex);
	int Message(MessageCode code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
	               TEdgeSegment* edge) override;

	static void TimerCallback(int timerId, void* caller);

	float TimerTime;
	int Timer;
};
