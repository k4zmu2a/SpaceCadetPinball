#pragma once
#include "TCollisionComponent.h"

class TFlipperEdge;

class TFlipper :
	public TCollisionComponent
{
public:
	TFlipper(TPinballTable* table, int groupIndex);
	~TFlipper() override;
	int Message(int code, float value) override;
	void port_draw() override;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
		TEdgeSegment* edge) override;

	static void TimerExpired(int timerId, void* caller);
	
	int BmpIndex;
	TFlipperEdge* FlipperEdge;
	int Timer;
	float ExtendAnimationFrameTime{};
	float RetractAnimationFrameTime{};
	float TimerTime{};
	float InputTime;
};
