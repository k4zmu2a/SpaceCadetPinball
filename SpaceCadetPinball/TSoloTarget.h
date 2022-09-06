#pragma once
#include "TCollisionComponent.h"

class TSoloTarget :
	public TCollisionComponent
{
public:
	TSoloTarget(TPinballTable* table, int groupIndex);
	int Message(MessageCode code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
	               TEdgeSegment* edge) override;

	static void TimerExpired(int timerId, void* caller);
	
	int Timer;
	float TimerTime;
	int SoundIndex4;
};
