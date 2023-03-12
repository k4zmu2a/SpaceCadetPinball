#include "pch.h"
#include "TEdgeManager.h"


#include "maths.h"
#include "TBall.h"
#include "TEdgeBox.h"
#include "TEdgeSegment.h"
#include "TTableLayer.h"

TEdgeManager::TEdgeManager(float xMin, float yMin, float width, float height)
{
	Width = width;
	Height = height;
	MinX = xMin;
	MinY = yMin;
	MaxX = MinX + width;
	MaxY = MinY + height;
	MaxBoxX = 10;
	MaxBoxY = 15;
	AdvanceX = width / static_cast<float>(MaxBoxX);
	AdvanceY = height / static_cast<float>(MaxBoxY);
	BoxArray = new TEdgeBox[MaxBoxX * MaxBoxY];
}

TEdgeManager::~TEdgeManager()
{
	delete[] BoxArray;
}

int TEdgeManager::box_x(float x)
{
	return std::max(0, std::min(static_cast<int>(floor((x - MinX) / AdvanceX)), MaxBoxX - 1));
}

int TEdgeManager::box_y(float y)
{
	return std::max(0, std::min(static_cast<int>(floor((y - MinY) / AdvanceY)), MaxBoxY - 1));
}

int TEdgeManager::increment_box_x(int x)
{
	return std::min(x + 1, MaxBoxX - 1);
}

int TEdgeManager::increment_box_y(int y)
{
	return std::min(y + 1, MaxBoxY - 1);
}

void TEdgeManager::add_edge_to_box(int x, int y, TEdgeSegment* edge)
{
	assertm((unsigned)x < (unsigned)MaxBoxX && (unsigned)y < (unsigned)MaxBoxY, "Box coordinates out of range");

	auto& list = BoxArray[x + y * MaxBoxX].EdgeList;
	assertm(std::find(list.begin(), list.end(), edge) == list.end(), "Duplicate inserted into box");
	list.push_back(edge);
}

void TEdgeManager::add_field_to_box(int x, int y, field_effect_type* field)
{
	assertm((unsigned)x < (unsigned)MaxBoxX && (unsigned)y < (unsigned)MaxBoxY, "Box coordinates out of range");

	auto& list = BoxArray[x + y * MaxBoxX].FieldList;
	assertm(std::find(list.begin(), list.end(), field) == list.end(), "Duplicate inserted into box");
	list.push_back(field);
}

int TEdgeManager::TestGridBox(int x, int y, float* distPtr, TEdgeSegment** edgeDst, ray_type* ray, TBall* ball,
                              int edgeIndex)
{
	if (x >= 0 && x < MaxBoxX && y >= 0 && y < MaxBoxY)
	{
		TEdgeBox* edgeBox = &BoxArray[x + y * MaxBoxX];
		TEdgeSegment** edgePtr = &EdgeArray[edgeIndex];
		for (auto it = edgeBox->EdgeList.rbegin(); it != edgeBox->EdgeList.rend(); ++it)
		{
			auto edge = *it;
			if (!edge->ProcessedFlag && *edge->ActiveFlagPtr && (edge->CollisionGroup & ray->CollisionMask))
			{
				if (!ball->already_hit(edge))
				{
					++edgeIndex;
					*edgePtr = edge;
					++edgePtr;
					edge->ProcessedFlag = 1;
					auto dist = edge->FindCollisionDistance(*ray);
					if (dist < *distPtr)
					{
						*distPtr = dist;
						*edgeDst = edge;
					}
				}
			}
		}
	}
	return edgeIndex;
}

void TEdgeManager::FieldEffects(TBall* ball, vector2* dstVec)
{
	vector2 vec{};
	TEdgeBox* edgeBox = &BoxArray[box_x(ball->Position.X) + box_y(ball->Position.Y) *
		MaxBoxX];

	for (auto it = edgeBox->FieldList.rbegin(); it != edgeBox->FieldList.rend(); ++it)
	{
		auto field = *it;
		if (*field->ActiveFlag && ball->CollisionMask & field->CollisionGroup)
		{
			if (field->CollisionComp->FieldEffect(ball, &vec))
			{
				maths::vector_add(*dstVec, vec);
			}
		}
	}
}

float TEdgeManager::FindCollisionDistance(ray_type* ray, TBall* ball, TEdgeSegment** edge)
{
	auto distance = 1000000000.0f;
	auto edgeIndex = 0;

	auto x0 = ray->Origin.X;
	auto y0 = ray->Origin.Y;
	auto x1 = ray->Direction.X * ray->MaxDistance + ray->Origin.X;
	auto y1 = ray->Direction.Y * ray->MaxDistance + ray->Origin.Y;

	auto xBox0 = box_x(x0);
	auto yBox0 = box_y(y0);
	auto xBox1 = box_x(x1);
	auto yBox1 = box_y(y1);

	auto dirX = x0 >= x1 ? -1 : 1;
	auto dirY = y0 >= y1 ? -1 : 1;

	if (yBox0 == yBox1)
	{
		if (dirX == 1)
		{
			for (auto indexX = xBox0; indexX <= xBox1; indexX++)
			{
				edgeIndex = TestGridBox(indexX, yBox0, &distance, edge, ray, ball, edgeIndex);
			}
		}
		else
		{
			for (auto indexX = xBox0; indexX >= xBox1; indexX--)
			{
				edgeIndex = TestGridBox(indexX, yBox0, &distance, edge, ray, ball, edgeIndex);
			}
		}
	}
	else if (xBox0 == xBox1)
	{
		if (dirY == 1)
		{
			for (auto indexY = yBox0; indexY <= yBox1; indexY++)
			{
				edgeIndex = TestGridBox(xBox0, indexY, &distance, edge, ray, ball, edgeIndex);
			}
		}
		else
		{
			for (auto indexY = yBox0; indexY >= yBox1; indexY--)
			{
				edgeIndex = TestGridBox(xBox0, indexY, &distance, edge, ray, ball, edgeIndex);
			}
		}
	}
	else
	{
		edgeIndex = TestGridBox(xBox0, yBox0, &distance, edge, ray, ball, 0);

		// Bresenham line formula: y = dYdX * (x - x0) + y0; dYdX = (y0 - y1) / (x0 - x1)
		auto dyDx = (y0 - y1) / (x0 - x1);
		// Precompute constant part: dYdX * (-x0) + y0
		auto precomp = -x0 * dyDx + y0;
		// X and Y indexes are offset by one when going forwards, not sure why
		auto xBias = dirX == 1 ? 1 : 0, yBias = dirY == 1 ? 1 : 0;

		for (auto indexX = xBox0, indexY = yBox0; indexX != xBox1 || indexY != yBox1;)
		{
			// Calculate y from indexY and from line formula
			auto yDiscrete = (indexY + yBias) * AdvanceY + MinY;
			auto ylinear = ((indexX + xBias) * AdvanceX + MinX) * dyDx + precomp;
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
			edgeIndex = TestGridBox(indexX, indexY, &distance, edge, ray, ball, edgeIndex);
		}
	}


	for (auto edgePtr = EdgeArray; edgeIndex > 0; --edgeIndex, ++edgePtr)
	{
		(*edgePtr)->ProcessedFlag = 0;
	}

	return distance;
}

vector2 TEdgeManager::NormalizeBox(vector2 pt) const
{
	// Standard PB Box ranges: X [-8, 8]; Y [-14, 15]; Top right corner: (-8, -14)
	// Bring them to: X [0, 16]; Y [0, 29]; Top right corner: (0, 0)
	auto x = Clamp(pt.X, MinX, MaxX) + abs(MinX);
	auto y = Clamp(pt.Y, MinY, MaxY) + abs(MinY);

	// Normalize and invert to: X [0, 1]; Y [0, 1]; Top right corner: (1, 1)
	x /= Width; y /= Height;
	return vector2{ 1 - x, 1 - y };
}

vector2 TEdgeManager::DeNormalizeBox(vector2 pt) const
{
	// Undo normalization by applying steps in reverse
	auto x = (1 - pt.X) * Width - abs(MinX);
	auto y = (1 - pt.Y) * Height - abs(MinY);
	return vector2{ x, y };
}
