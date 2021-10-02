#pragma once

#include "TCollisionComponent.h"

struct gdrv_bitmap8;

class TWall :
	public TCollisionComponent
{
public:
	TWall(TPinballTable* table, int groupIndex);
	int Message(int code, float value) override;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
	               TEdgeSegment* edge) override;
	void put_scoring(int index, int score) override;
	int get_scoring(int index) override;

	static void TimerExpired(int timerId, void* caller);

	int Timer{};
	gdrv_bitmap8* BmpPtr{};
	int Scores[1]{};
};
