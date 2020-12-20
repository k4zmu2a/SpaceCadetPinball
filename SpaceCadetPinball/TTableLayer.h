#pragma once
#include "TCollisionComponent.h"
#include "TEdgeManager.h"




class TTableLayer :
	public TCollisionComponent
{
public:
	static TEdgeManager *edge_manager;
	
	TTableLayer(TPinballTable* table);
	~TTableLayer() override;

	gdrv_bitmap8* VisBmp;
	float Unknown1F;
	float Unknown2F;
	float Unknown3F;
	float Unknown4F;
	float Angle1;
	float Angle2;
	int Unknown7;
	float AngleMult;
	field_effect_type Field;
};
