#pragma once
#include "TCollisionComponent.h"

class TFlagSpinner :
	public TCollisionComponent
{
public:
	TFlagSpinner(TPinballTable* table, int groupIndex);
	int Message(int code, float value) override;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
		TEdgeSegment* edge) override;
	void put_scoring(int index, int score) override;
	int get_scoring(int index) override;
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
	int Scores[2]{};
};

