#pragma once
#include "TRollover.h"

class TTripwire :
	public TRollover
{
public:
	TTripwire(TPinballTable* table, int groupIndex);
	~TTripwire() override = default;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
	               TEdgeSegment* edge) override;
};
