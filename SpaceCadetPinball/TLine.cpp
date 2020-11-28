#include "pch.h"
#include "TLine.h"


TLine::TLine(TCollisionComponent* collCmp, char* flagPtr, unsigned int visualFlag, float x0, float y0, float x1,
             float y1): TEdgeSegment(collCmp, flagPtr, visualFlag)
{
	this->X0 = x0;
	this->Y0 = y0;
	this->X1 = x1;
	this->Y1 = y1;
	maths::line_init(&Line, x0, y0, x1, y1);
}

TLine::TLine(TCollisionComponent* collCmp, char* flagPtr, unsigned int visualFlag, struct vector_type* start,
             struct vector_type* end) : TEdgeSegment(collCmp, flagPtr, visualFlag)
{
	this->X0 = start->X;
	this->Y0 = start->Y;
	this->X1 = end->X;
	this->Y1 = end->Y;
	maths::line_init(&Line, X0, Y0, X1, Y1);
}

void TLine::Offset(float offset)
{
	float offX = offset * Line.PerpendicularL.X;
	float offY = offset * Line.PerpendicularL.Y;

	X0 += offX;
	Y0 += offY;
	X1 += offX;
	Y1 += offY;
	maths::line_init(&Line, X0, Y0, X1, Y1);
}

double TLine::FindCollisionDistance(ray_type* ray)
{
	return maths::ray_intersect_line(ray, &Line);
}
