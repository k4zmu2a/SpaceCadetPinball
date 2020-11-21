#pragma once
#include "maths.h"
#include "TEdgeSegment.h"

class TLine :
	public TEdgeSegment
{
public:
	line_type Line;
	vector_type Start;
	vector_type End;
	TLine(TCollisionComponent* collCmp, char* flagPtr, unsigned int visualFlag, float x0, float y0, float x1, float y1);
	TLine(TCollisionComponent* collCmp, char* flagPtr, unsigned int visualFlag, struct vector_type* start,
	      struct vector_type* end);
	void Offset(float offset);
	double FindCollisionDistance(ray_type* ray) override;
};
