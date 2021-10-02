#pragma once
#include "TCollisionComponent.h"

class TRollover :
	public TCollisionComponent
{
protected:
	TRollover(TPinballTable* table, int groupIndex, bool createWall);
public:
	TRollover(TPinballTable* table, int groupIndex);
	~TRollover() override = default;
	int Message(int code, float value) override;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
		TEdgeSegment* edge) override;
	void put_scoring(int index, int score) override;
	int get_scoring(int index) override;
	void build_walls(int groupIndex);

	static void TimerExpired(int timerId, void* caller);

	char RolloverFlag{};
	int Scores[2]{};
};
