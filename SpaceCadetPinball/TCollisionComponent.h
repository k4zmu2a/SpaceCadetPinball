#pragma once
#include "TPinballComponent.h"
#include "TBall.h"

struct vector2;
class TEdgeSegment;

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
	virtual void Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance,
	                       TEdgeSegment* edge);
	virtual int FieldEffect(TBall* ball, vector2* vecDst);
	bool DefaultCollision(TBall* ball, vector2* nextPosition, vector2* direction);
};
