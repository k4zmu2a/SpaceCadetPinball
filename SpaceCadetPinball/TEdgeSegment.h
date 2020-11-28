#pragma once
#include "TCollisionComponent.h"
#include "maths.h"
#include "TBall.h"

enum class wall_type
{
	Circle = 0,
	Line = 1,
};

class TEdgeSegment
{
public:
	TCollisionComponent* CollisionComponent;
	char* PinbCompFlag2Ptr;
	char Unknown3_0;
	int WallValue;
	int VisualFlag;

	TEdgeSegment(TCollisionComponent* collComp, char* someFlag, unsigned int visualFlag);

	virtual ~TEdgeSegment()
	{
	}

	virtual void place_in_grid() = 0;
	virtual double FindCollisionDistance(ray_type* ray) = 0;
	virtual void EdgeCollision(TBall* ball, float coef) = 0;
	static TEdgeSegment* install_wall(float* floatArr, TCollisionComponent* collComp, char* flagPtr,
	                                  unsigned int visual_flag, float offset, int someValue);
};
