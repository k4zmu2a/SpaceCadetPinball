#include "pch.h"
#include "TCircle.h"

TCircle::TCircle(TCollisionComponent* collComp, char* someFlagPtr, unsigned visualFlag, vector_type* center,
                 float radius): TEdgeSegment(collComp, someFlagPtr, visualFlag)
{
	Circle.RadiusSq = radius * radius;
	Circle.Center = *center;
}

double TCircle::FindCollisionDistance(ray_type* ray)
{
	return maths::ray_intersect_circle(ray, &Circle);
}