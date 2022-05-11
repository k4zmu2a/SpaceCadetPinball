#pragma once

#include "TCollisionComponent.h"
#include "TEdgeManager.h"

struct ramp_plane_type;

class TRamp :
	public TCollisionComponent
{
public:
	TRamp(TPinballTable* table, int groupIndex);
	void put_scoring(int index, int score) override;
	int get_scoring(int index) override;
	void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float coef,
	               TEdgeSegment* edge) override;
	int FieldEffect(TBall* ball, vector2* vecDst) override;
	void port_draw() override;

	int Scores[4]{};
	field_effect_type Field{};
	int CollisionGroup;
	int RampFlag1;
	int RampPlaneCount;
	float BallFieldMult;
	ramp_plane_type* RampPlane;
	TEdgeSegment* Line2;
	TEdgeSegment* Line3;
	TEdgeSegment* Line1;
	int Wall1PointFirst;
	int Wall2PointFirst;
	float Wall1PointLast;
	float Wall2PointLast;
};
