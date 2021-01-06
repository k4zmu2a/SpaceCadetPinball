#pragma once
#include "objlist_class.h"
#include "TPinballComponent.h"

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
	float UnknownC6F;
	float UnknownC7F;
	int SoundIndex2;
	int SoundIndex1;

	TCollisionComponent(TPinballTable* table, int groupIndex, bool createWall);
	~TCollisionComponent();
	void port_draw() override;
	virtual void Collision(TBall* ball, struct vector_type* ballPosition, struct vector_type* vec2, float someVal,
	                       TEdgeSegment* edge);
	virtual int FieldEffect(TBall* ball, struct vector_type* vecDst);
	int DefaultCollision(TBall* ball, struct vector_type* ballPosition, struct vector_type* vec2);
};
