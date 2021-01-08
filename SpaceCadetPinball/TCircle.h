#pragma once
#include "maths.h"
#include "TEdgeSegment.h"

class TCircle :
	public TEdgeSegment
{
public:
	circle_type Circle;

	TCircle(TCollisionComponent* collComp, char* someFlagPtr, unsigned int visualFlag, vector_type* center,
	        float radius);
	float FindCollisionDistance(ray_type* ray) override;
	void EdgeCollision(TBall* ball, float coef) override;
	void place_in_grid() override;
};
