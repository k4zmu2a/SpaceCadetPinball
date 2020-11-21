#pragma once
#include "maths.h"
#include "TEdgeSegment.h"

class TCircle :
	public TEdgeSegment
{
	circle_type Circle;

	TCircle(TCollisionComponent* collComp, char* someFlagPtr, unsigned int visualFlag, float* floatArr, float radius);
	double FindCollisionDistance(ray_type* ray) override;
};
