#pragma once
#include "TCollisionComponent.h"

class TDemo :
	public TCollisionComponent
{
public:
	TDemo(TPinballTable* table, int groupIndex);
	int Message(MessageCode code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
	               TEdgeSegment* edge) override;

	static void PlungerRelease(int timerId, void* caller);
	static void UnFlipRight(int timerId, void* caller);
	static void UnFlipLeft(int timerId, void* caller);
	static void FlipRight(int timerId, void* caller);
	static void FlipLeft(int timerId, void* caller);
	static void NewGameRestartTimer(int timerId, void* caller);

	float FlipTimerTime1;
	float FlipTimerTime2;
	float UnFlipTimerTime1;
	float UnFlipTimerTime2;
	int FlipLeftFlag;
	int FlipRightFlag;
	int FlipLeftTimer;
	int FlipRightTimer;
	int PlungerFlag;
	int RestartGameTimer;
	TEdgeSegment* Edge1;
	TEdgeSegment* Edge2;
	TEdgeSegment* Edge3;
};
