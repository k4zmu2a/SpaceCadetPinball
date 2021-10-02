#pragma once
#include "TCollisionComponent.h"

struct ray_type;
class TEdgeBox;

struct field_effect_type
{
	char* Flag2Ptr;
	int Mask;
	TCollisionComponent* CollisionComp;
};

class TEdgeManager
{
public:
	TEdgeManager(float posX, float posY, float width, float height);
	~TEdgeManager();
	void FieldEffects(TBall* ball, struct vector_type* dstVec);
	int box_x(float x);
	int box_y(float y);
	int increment_box_x(int x);
	int increment_box_y(int y);
	void add_edge_to_box(int x, int y, TEdgeSegment* edge);
	void add_field_to_box(int x, int y, field_effect_type* field);
	int TestGridBox(int x, int y, float* distPtr, TEdgeSegment** edgeDst, ray_type* ray, TBall* ball, int edgeIndex);
	float FindCollisionDistance(ray_type* ray, TBall* ball, TEdgeSegment** edge);

	float AdvanceX;
	float AdvanceY;
	float AdvanceXInv;
	float AdvanceYInv;
	int MaxBoxX;
	int MaxBoxY;
	float X;
	float Y;
	TEdgeBox* BoxArray;
	TEdgeSegment* EdgeArray[1000]{};
};
