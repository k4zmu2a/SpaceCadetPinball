#pragma once
#include "TCollisionComponent.h"
#include "maths.h"
#include "TBall.h"

class TEdgeSegment
{
public:
	TCollisionComponent* CollisionComponent;
	char* PinbCompFlag2Ptr;
	char Unknown3_0;
	char Unknown3_1;
	char Unknown3_2;
	char Unknown3_3;
	char Unknown4;
	int VisualFlag;

	TEdgeSegment(TCollisionComponent* collComp, char* someFlag, unsigned int visualFlag);
	//virtual ~TEdgeSegment() = 0;
	virtual void place_in_grid() = 0;
	virtual double FindCollisionDistance(ray_type* ray) = 0;
	virtual void EdgeCollision(TBall* ball, float coef) = 0;
};
