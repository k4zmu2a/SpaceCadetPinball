#include "pch.h"
#include "TLine.h"

#include "TTableLayer.h"


TLine::TLine(TCollisionComponent* collCmp, char* activeFlag, unsigned int collisionGroup, float x0, float y0, float x1,
             float y1): TEdgeSegment(collCmp, activeFlag, collisionGroup)
{
	X0 = x0;
	Y0 = y0;
	X1 = x1;
	Y1 = y1;
	maths::line_init(Line, x0, y0, x1, y1);
}

TLine::TLine(TCollisionComponent* collCmp, char* activeFlag, unsigned int collisionGroup, const vector2& start,
             const vector2& end) : TEdgeSegment(collCmp, activeFlag, collisionGroup)
{
	X0 = start.X;
	Y0 = start.Y;
	X1 = end.X;
	Y1 = end.Y;
	maths::line_init(Line, X0, Y0, X1, Y1);
}

void TLine::Offset(float offset)
{
	float offX = offset * Line.PerpendicularC.X;
	float offY = offset * Line.PerpendicularC.Y;

	X0 += offX;
	Y0 += offY;
	X1 += offX;
	Y1 += offY;
	maths::line_init(Line, X0, Y0, X1, Y1);
}

float TLine::FindCollisionDistance(ray_type* ray)
{
	return maths::ray_intersect_line(*ray, Line);
}

void TLine::EdgeCollision(TBall* ball, float distance)
{
	CollisionComponent->Collision(
		ball,
		&Line.RayIntersect,
		&Line.PerpendicularC,
		distance,
		this);
}

void TLine::place_in_grid(RectF* aabb)
{
	if (aabb)
	{
		aabb->Merge({
			std::max(X0, X1), std::max(Y0, Y1),
			std::min(X0, X1), std::min(Y0, Y1)
		});
	}

	auto edgeMan = TTableLayer::edge_manager;
	auto xBox0 = edgeMan->box_x(X0);
	auto yBox0 = edgeMan->box_y(Y0);
	auto xBox1 = edgeMan->box_x(X1);
	auto yBox1 = edgeMan->box_y(Y1);

	int dirX = X0 >= X1 ? -1 : 1;
	int dirY = Y0 >= Y1 ? -1 : 1;

	if (yBox0 == yBox1)
	{
		if (dirX == 1)
		{
			while (xBox0 <= xBox1)
				edgeMan->add_edge_to_box(xBox0++, yBox0, this);
		}
		else
		{
			while (xBox0 >= xBox1)
				edgeMan->add_edge_to_box(xBox0--, yBox0, this);
		}
	}
	else if (xBox0 == xBox1)
	{
		if (dirY == 1)
		{
			while (yBox0 <= yBox1)
				edgeMan->add_edge_to_box(xBox0, yBox0++, this);
		}
		else
		{
			while (yBox0 >= yBox1)
				edgeMan->add_edge_to_box(xBox0, yBox0--, this);
		}
	}
	else
	{
		edgeMan->add_edge_to_box(xBox0, yBox0, this);

		// Bresenham line formula: y = dYdX * (x - x0) + y0; dYdX = (y0 - y1) / (x0 - x1)
		auto dyDx = (Y0 - Y1) / (X0 - X1);
		// Precompute constant part: dYdX * (-x0) + y0
		auto precomp = -X0 * dyDx + Y0;
		// X and Y indexes are offset by one when going forwards, not sure why
		auto xBias = dirX == 1 ? 1 : 0, yBias = dirY == 1 ? 1 : 0;

		for (auto indexX = xBox0, indexY = yBox0; indexX != xBox1 || indexY != yBox1;)
		{
			// Calculate y from indexY and from line formula
			auto yDiscrete = (indexY + yBias) * edgeMan->AdvanceY + edgeMan->MinY;
			auto ylinear = ((indexX + xBias) * edgeMan->AdvanceX + edgeMan->MinX) * dyDx + precomp;
			if (dirY == 1 ? ylinear >= yDiscrete : ylinear <= yDiscrete)
			{
				// Advance indexY when discrete value is ahead/behind
				// Advance indexX when discrete value matches linear value
				indexY += dirY;
				if (ylinear == yDiscrete)
					indexX += dirX;
			}
			else
			{
				// Advance indexX otherwise
				indexX += dirX;
			}
			edgeMan->add_edge_to_box(indexX, indexY, this);
		}
	}
}
