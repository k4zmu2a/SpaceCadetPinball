#pragma once
#include "TCollisionComponent.h"

class TPlunger :
	public TCollisionComponent
{
public:
	TPlunger(TPinballTable* table, int groupIndex);
	~TPlunger() override = default;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
	               TEdgeSegment* edge) override;
	int Message(int code, float value) override;

	static void BallFeedTimer(int timerId, void* caller);
	static void PullbackTimer(int timerId, void* caller);
	static void PlungerReleasedTimer(int timerId, void* caller);

	int PullbackTimer_;
	int BallFeedTimer_;
	int MaxPullback;
	int PullbackIncrement;
	float Unknown4F;
	int SoundIndexP1;
	int SoundIndexP2;
};
