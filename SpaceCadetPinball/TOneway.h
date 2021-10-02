#pragma once
#include "TCollisionComponent.h"

class TLine;

class TOneway : public TCollisionComponent
{
public:
	TOneway(TPinballTable* table, int groupIndex);
	~TOneway() override = default;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
	               TEdgeSegment* edge) override;
	void put_scoring(int index, int score) override;
	int get_scoring(int index) override;

	TLine* Line;
	int Scores[6]{};
};
