#pragma once
#include "TPinballComponent.h"

struct vector_type;
class TEdgeSegment;
class TBall;

class TCollisionComponent : public TPinballComponent
{
public:
	std::vector<TEdgeSegment*> EdgeList;
	float Elasticity;
	float Smoothness;
	float Boost;
	float Threshold;
	int SoftHitSoundId;
	int HardHitSoundId;

	TCollisionComponent(TPinballTable* table, int groupIndex, bool createWall);
	~TCollisionComponent() override;
	void port_draw() override;
	virtual void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
	                       TEdgeSegment* edge);
	virtual int FieldEffect(TBall* ball, vector_type* vecDst);
	int DefaultCollision(TBall* ball, vector_type* nextPosition, vector_type* direction);
};
