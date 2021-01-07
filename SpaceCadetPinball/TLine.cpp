#include "pch.h"
#include "TLine.h"

#include "TTableLayer.h"


TLine::TLine(TCollisionComponent* collCmp, char* flagPtr, unsigned int visualFlag, float x0, float y0, float x1,
             float y1): TEdgeSegment(collCmp, flagPtr, visualFlag)
{
	X0 = x0;
	Y0 = y0;
	X1 = x1;
	Y1 = y1;
	maths::line_init(&Line, x0, y0, x1, y1);
}

TLine::TLine(TCollisionComponent* collCmp, char* flagPtr, unsigned int visualFlag, struct vector_type* start,
             struct vector_type* end) : TEdgeSegment(collCmp, flagPtr, visualFlag)
{
	X0 = start->X;
	Y0 = start->Y;
	X1 = end->X;
	Y1 = end->Y;
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

float TLine::FindCollisionDistance(ray_type* ray)
{
	return maths::ray_intersect_line(ray, &Line);
}

void TLine::EdgeCollision(TBall* ball, float coef)
{
	CollisionComponent->Collision(
		ball,
		&Line.RayIntersect,
		&Line.PerpendicularL,
		coef,
		this);
}

void TLine::place_in_grid()
{
	auto xBox0 = TTableLayer::edge_manager->box_x(X0);
	auto yBox0 = TTableLayer::edge_manager->box_y(Y0);
	auto xBox1 = TTableLayer::edge_manager->box_x(X1);
	auto yBox1 = TTableLayer::edge_manager->box_y(Y1);

	int dirX = X0 >= X1 ? -1 : 1;
	int dirY = Y0 >= Y1 ? -1 : 1;

	if (yBox0 == yBox1)
	{
		if (dirX == 1)
		{
			while (xBox0 <= xBox1)
				TTableLayer::edge_manager->add_edge_to_box(xBox0++, yBox0, this);
		}
		else
		{
			while (xBox0 >= xBox1)
				TTableLayer::edge_manager->add_edge_to_box(xBox0--, yBox0, this);
		}
	}
	else if (xBox0 == xBox1)
	{
		if (dirY == 1)
		{
			if (yBox0 <= yBox1)
			{
				do
					TTableLayer::edge_manager->add_edge_to_box(xBox0, yBox0++, this);
				while (yBox0 <= yBox1);
			}
		}
		else if (yBox0 >= yBox1)
		{
			do
				TTableLayer::edge_manager->add_edge_to_box(xBox0, yBox0--, this);
			while (yBox0 >= yBox1);
		}
	}
	else
	{
		float yCoord, xCoord;
		int indexX1 = xBox0, indexY1 = yBox0;
		int bresIndexX = xBox0 + 1, bresIndexY = yBox0 + 1;
		auto bresDyDx = (Y0 - Y1) / (X0 - X1);
		auto bresXAdd = Y0 - bresDyDx * X0;
		TTableLayer::edge_manager->add_edge_to_box(xBox0, yBox0, this);
		if (dirX == 1)
		{
			if (dirY == 1)
			{
				do
				{
					yCoord = bresIndexY * TTableLayer::edge_manager->AdvanceY + TTableLayer::edge_manager->Y;
					xCoord = (bresIndexX * TTableLayer::edge_manager->AdvanceX + TTableLayer::edge_manager->X) *
						bresDyDx + bresXAdd;
					if (xCoord >= yCoord)
					{
						if (xCoord == yCoord)
						{
							++indexX1;
							++bresIndexX;
						}
						++indexY1;
						++bresIndexY;
					}
					else
					{
						++indexX1;
						++bresIndexX;
					}
					TTableLayer::edge_manager->add_edge_to_box(indexX1, indexY1, this);
				}
				while (indexX1 != xBox1 || indexY1 != yBox1);
			}
			else
			{
				do
				{
					yCoord = indexY1 * TTableLayer::edge_manager->AdvanceY + TTableLayer::edge_manager->Y;
					xCoord = (bresIndexX * TTableLayer::edge_manager->AdvanceX + TTableLayer::edge_manager->X) *
						bresDyDx + bresXAdd;
					if (xCoord <= yCoord)
					{
						if (xCoord == yCoord)
						{
							++indexX1;
							++bresIndexX;
						}
						--indexY1;
					}
					else
					{
						++indexX1;
						++bresIndexX;
					}
					TTableLayer::edge_manager->add_edge_to_box(indexX1, indexY1, this);
				}
				while (indexX1 != xBox1 || indexY1 != yBox1);
			}
		}
		else
		{
			if (dirY == 1)
			{
				do
				{
					xCoord = bresIndexY * TTableLayer::edge_manager->AdvanceY + TTableLayer::edge_manager->Y;
					yCoord = (indexX1 * TTableLayer::edge_manager->AdvanceX + TTableLayer::edge_manager->X) *
						bresDyDx + bresXAdd;
					if (yCoord >= xCoord)
					{
						if (yCoord == xCoord)
							--indexX1;
						++indexY1;
						++bresIndexY;
					}
					else
					{
						--indexX1;
					}
					TTableLayer::edge_manager->add_edge_to_box(indexX1, indexY1, this);
				}
				while (indexX1 != xBox1 || indexY1 != yBox1);
			}
			else
			{
				do
				{
					yCoord = indexY1 * TTableLayer::edge_manager->AdvanceY + TTableLayer::edge_manager->Y;
					xCoord = (indexX1 * TTableLayer::edge_manager->AdvanceX + TTableLayer::edge_manager->X) *
						bresDyDx + bresXAdd;
					if (xCoord <= yCoord)
					{
						if (xCoord == yCoord)
							--indexX1;
						--indexY1;
					}
					else
					{
						--indexX1;
					}
					TTableLayer::edge_manager->add_edge_to_box(indexX1, indexY1, this);
				}
				while (indexX1 != xBox1 || indexY1 != yBox1);
			}
		}
	}
}
