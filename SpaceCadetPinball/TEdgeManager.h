#pragma once
#include "TCollisionComponent.h"

struct ray_type;
class TEdgeBox;

struct field_effect_type
{
	char* ActiveFlag;
	int CollisionGroup;
	TCollisionComponent* CollisionComp;
};

class TEdgeManager
{
public:
	TEdgeManager(float xMin, float yMin, float width, float height);
	~TEdgeManager();
	void FieldEffects(TBall* ball, struct vector2* dstVec);
	int box_x(float x);
	int box_y(float y);
	int increment_box_x(int x);
	int increment_box_y(int y);
	void add_edge_to_box(int x, int y, TEdgeSegment* edge);
	void add_field_to_box(int x, int y, field_effect_type* field);
	int TestGridBox(int x, int y, float* distPtr, TEdgeSegment** edgeDst, ray_type* ray, TBall* ball, int edgeIndex);
	float FindCollisionDistance(ray_type* ray, TBall* ball, TEdgeSegment** edge);
	vector2 NormalizeBox(vector2 pt) const;
	vector2 DeNormalizeBox(vector2 pt) const;

	float AdvanceX;
	float AdvanceY;
	int MaxBoxX;
	int MaxBoxY;
	float MinX;
	float MinY;
	float MaxX;
	float MaxY;
	float Width;
	float Height;
	TEdgeBox* BoxArray;
	TEdgeSegment* EdgeArray[1000]{};
};
