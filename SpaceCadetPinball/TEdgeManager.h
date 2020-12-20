#pragma once
#include "TCollisionComponent.h"
#include "TEdgeSegment.h"

struct field_effect_type
{
	char* Flag2Ptr;
	int Unknown1;
	TCollisionComponent* CollisionComp;
};

class TEdgeManager
{
public:
	TEdgeManager(float a2, float a3, float a4, float a5)
	{
	}

	static void edges_insert_square(float a1, float a2, float a3, float a4, TEdgeSegment* a5, field_effect_type* a6);
};
