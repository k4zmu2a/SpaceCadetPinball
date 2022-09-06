#pragma once
#include "TCollisionComponent.h"

class TKickback :
	public TCollisionComponent
{
public:
	TKickback(TPinballTable* table, int groupIndex);
	int Message(MessageCode code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
		TEdgeSegment* edge) override;

	static void TimerExpired(int timerId, void* caller);

	float TimerTime;
	float TimerTime2;
	int Timer;
	int KickActiveFlag;
};
