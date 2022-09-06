#pragma once
#include "TCollisionComponent.h"

class TFlagSpinner :
	public TCollisionComponent
{
public:
	TFlagSpinner(TPinballTable* table, int groupIndex);
	int Message(MessageCode code, float value) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
		TEdgeSegment* edge) override;
	void NextFrame();

	static void SpinTimer(int timerId, void* caller);

	float Speed{};
	float MaxSpeed;
	float MinSpeed;
	float SpeedDecrement;
	int SpinDirection{};
	int BmpIndex{};
	int Timer;
	TEdgeSegment* PrevCollider;
};

