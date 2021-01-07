#pragma once
#include "maths.h"
#include "TEdgeSegment.h"

class TLine :
	public TEdgeSegment
{
public:
	line_type Line;
	float X0, Y0, X1, Y1;
	TLine(TCollisionComponent* collCmp, char* flagPtr, unsigned int visualFlag, float x0, float y0, float x1, float y1);
	TLine(TCollisionComponent* collCmp, char* flagPtr, unsigned int visualFlag, struct vector_type* start,
	      struct vector_type* end);
	void Offset(float offset);
	float FindCollisionDistance(ray_type* ray) override;
	void EdgeCollision(TBall* ball, float coef) override;
	void place_in_grid() override;
};
