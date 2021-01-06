#include "pch.h"
#include "TEdgeManager.h"

int TEdgeManager::FieldEffects(TBall* ball, vector_type* vecDst)
{
	return 0;
}

void TEdgeManager::edges_insert_square(float a1, float a2, float a3, float a4, TEdgeSegment* a5, field_effect_type* a6)
{
}

float TEdgeManager::FindCollisionDistance(ray_type* ray, TBall* ball, TEdgeSegment** edge)
{
	return 1000000000.0;
}
