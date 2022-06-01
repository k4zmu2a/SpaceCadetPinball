#pragma once
#include "TCollisionComponent.h"
#include "TEdgeManager.h"


struct circle_type;
class TPinballTable;
class TEdgeManager;
struct gdrv_bitmap8;

class TTableLayer :
	public TCollisionComponent
{
public:
	TTableLayer(TPinballTable* table);
	~TTableLayer() override;
	int FieldEffect(TBall* ball, vector2* vecDst) override;

	static void edges_insert_square(float y0, float x0, float y1, float x1, TEdgeSegment* edge,
	                                field_effect_type* field);
	static void edges_insert_circle(circle_type* circle, TEdgeSegment* edge, field_effect_type* field);

	gdrv_bitmap8* VisBmp;
	float XMin;
	float YMin;
	float XMax;
	float YMax;
	float GraityDirX;
	float GraityDirY;
	float GraityMult;
	field_effect_type Field{};

	static TEdgeManager* edge_manager;
};
