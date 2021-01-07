#pragma once
#include "objlist_class.h"
#include "TPinballComponent.h"

struct vector_type;
class TEdgeSegment;
class TBall;

class TCollisionComponent : public TPinballComponent
{
public:
	objlist_class* EdgeList;
	__int16 UnknownC2;
	__int16 UnknownC3;
	float UnknownC4F;
	float UnknownC5F;
	float CollisionMultiplier;
	float MaxCollisionSpeed;
	int SoundIndex2;
	int SoundIndex1;

	TCollisionComponent(TPinballTable* table, int groupIndex, bool createWall);
	~TCollisionComponent();
	void port_draw() override;
	virtual void Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef,
	                       TEdgeSegment* edge);
	virtual int FieldEffect(TBall* ball, vector_type* vecDst);
	int DefaultCollision(TBall* ball, vector_type* nextPosition, vector_type* direction);
};
