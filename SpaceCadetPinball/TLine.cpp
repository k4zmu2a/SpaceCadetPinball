#include "pch.h"
#include "TLine.h"


TLine::TLine(TCollisionComponent* collCmp, char* flagPtr, unsigned int visualFlag, float x0, float y0, float x1,
             float y1): TEdgeSegment(collCmp, flagPtr, visualFlag)
{
	Start.X = x0;
	Start.Y = y0;
	End.X = x1;
	End.Y = y1;
	maths::line_init(&Line, x0, y0, x1, y1);
}

TLine::TLine(TCollisionComponent* collCmp, char* flagPtr, unsigned int visualFlag, struct vector_type* start,
             struct vector_type* end) : TEdgeSegment(collCmp, flagPtr, visualFlag)
{
	Start = *start;
	End = *end;
	maths::line_init(&Line, Start.X, Start.Y, End.X, End.Y);
}

void TLine::Offset(float offset)
{
	float offX = offset * Line.PerpendicularL.X;
	float offY = offset * Line.PerpendicularL.Y;

	Start.X += offX;
	Start.Y += offY;
	End.X += offX;
	End.Y += offY;
	maths::line_init(&Line, Start.X, Start.Y, End.X, End.Y);
}

double TLine::FindCollisionDistance(ray_type* ray)
{
	return maths::ray_intersect_line(ray, &Line);
}