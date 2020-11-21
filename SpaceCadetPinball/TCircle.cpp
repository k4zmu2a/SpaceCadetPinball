#include "pch.h"
#include "TCircle.h"

TCircle::TCircle(TCollisionComponent* collComp, char* someFlagPtr, unsigned visualFlag, float* floatArr,
                 float radius): TEdgeSegment(collComp, someFlagPtr, visualFlag)
{
	Circle.RadiusSq = radius * radius;
	Circle.X = *floatArr;
	Circle.Y = floatArr[1];
	Circle.Unknown2 = floatArr[2];
}

double TCircle::FindCollisionDistance(ray_type* ray)
{
	return maths::ray_intersect_circle(ray, &Circle);
}