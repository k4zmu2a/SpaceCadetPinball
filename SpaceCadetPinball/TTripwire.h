#pragma once
#include "TRollover.h"

class TTripwire :
	public TRollover
{
public:
	TTripwire(TPinballTable* table, int groupIndex);
	~TTripwire() override = default;
	void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
	               TEdgeSegment* edge) override;
};
