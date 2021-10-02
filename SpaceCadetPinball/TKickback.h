#pragma once
#include "TCollisionComponent.h"

class TKickback :
	public TCollisionComponent
{
public:
	TKickback(TPinballTable* table, int groupIndex);
	int Message(int code, float value) override;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
		TEdgeSegment* edge) override;

	static void TimerExpired(int timerId, void* caller);

	float TimerTime;
	float TimerTime2;
	int Timer;
	int KickActiveFlag;
};
