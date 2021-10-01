#include "pch.h"
#include "TEdgeManager.h"


#include "maths.h"
#include "TBall.h"
#include "TEdgeBox.h"
#include "TEdgeSegment.h"
#include "TTableLayer.h"

TEdgeManager::TEdgeManager(float posX, float posY, float width, float height)
{
	X = posX;
	Y = posY;
	MaxBoxX = 10;
	MaxBoxY = 15;
	AdvanceX = width / 10.0f;
	AdvanceY = height / 15.0f;
	AdvanceXInv = 1.0f / AdvanceX;
	AdvanceYInv = 1.0f / AdvanceY;
	BoxArray = new TEdgeBox[150];
}

TEdgeManager::~TEdgeManager()
{
	delete[] BoxArray;
}

int TEdgeManager::box_x(float x)
{
	return std::max(0, std::min(static_cast<int>(floor((x - X) * AdvanceXInv)), MaxBoxX - 1));
}

int TEdgeManager::box_y(float y)
{
	return std::max(0, std::min(static_cast<int>(floor((y - Y) * AdvanceYInv)), MaxBoxY - 1));
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
	BoxArray[x + y * MaxBoxX].EdgeList.push_back(edge);
}

void TEdgeManager::add_field_to_box(int x, int y, field_effect_type* field)
{
	BoxArray[x + y * MaxBoxX].FieldList.push_back(field);
}

int TEdgeManager::TestGridBox(int x, int y, float* distPtr, TEdgeSegment** edgeDst, ray_type* ray, TBall* ball,
                              int edgeIndex)
{
	if (x >= 0 && x < 10 && y >= 0 && y < 15)
	{
		TEdgeBox* edgeBox = &BoxArray[x + y * MaxBoxX];
		TEdgeSegment** edgePtr = &EdgeArray[edgeIndex];
		for (auto it = edgeBox->EdgeList.rbegin(); it != edgeBox->EdgeList.rend(); ++it)
		{
			auto edge = *it;
			if (!edge->ProcessedFlag && *edge->ActiveFlag && (edge->CollisionGroup & ray->FieldFlag))
			{
				if (!ball->already_hit(edge))
				{
					++edgeIndex;
					*edgePtr = edge;
					++edgePtr;
					edge->ProcessedFlag = 1;
					auto dist = edge->FindCollisionDistance(ray);
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

void TEdgeManager::FieldEffects(TBall* ball, vector_type* dstVec)
{
	vector_type vec{};
	TEdgeBox* edgeBox = &BoxArray[box_x(ball->Position.X) + box_y(ball->Position.Y) *
		MaxBoxX];

	for (auto it = edgeBox->FieldList.rbegin(); it != edgeBox->FieldList.rend(); ++it)
	{
		auto field = *it;
		if (*field->Flag2Ptr && ball->FieldFlag & field->Mask)
		{
			if (field->CollisionComp->FieldEffect(ball, &vec))
			{
				maths::vector_add(dstVec, &vec);
			}
		}
	}
}

float TEdgeManager::FindCollisionDistance(ray_type* ray, TBall* ball, TEdgeSegment** edge)
{
	auto distance = 1000000000.0f;
	auto edgeIndex = 0;

	auto rayX = ray->Origin.X;
	auto rayY = ray->Origin.Y;
	auto rayBoxX = box_x(rayX);
	auto rayBoxY = box_y(rayY);

	auto rayEndX = ray->Direction.X * ray->MaxDistance + ray->Origin.X;
	auto rayEndY = ray->Direction.Y * ray->MaxDistance + ray->Origin.Y;
	auto rayEndBoxX = box_x(rayEndX);
	auto rayEndBoxY = box_y(rayEndY);

	auto rayDirX = rayX >= rayEndX ? -1 : 1;
	auto rayDirY = rayY >= rayEndY ? -1 : 1;

	if (rayBoxY == rayEndBoxY)
	{
		if (rayDirX == 1)
		{
			for (auto indexX = rayBoxX; indexX <= rayEndBoxX; indexX++)
			{
				edgeIndex = TestGridBox(indexX, rayBoxY, &distance, edge, ray, ball, edgeIndex);
			}
		}
		else
		{
			for (auto indexX = rayBoxX; indexX >= rayEndBoxX; indexX--)
			{
				edgeIndex = TestGridBox(indexX, rayBoxY, &distance, edge, ray, ball, edgeIndex);
			}
		}
	}
	else if (rayBoxX == rayEndBoxX)
	{
		if (rayDirY == 1)
		{
			for (auto indexY = rayBoxY; indexY <= rayEndBoxY; indexY++)
			{
				edgeIndex = TestGridBox(rayBoxX, indexY, &distance, edge, ray, ball, edgeIndex);
			}
		}
		else
		{
			for (auto indexY = rayBoxY; indexY >= rayEndBoxY; indexY--)
			{
				edgeIndex = TestGridBox(rayBoxX, indexY, &distance, edge, ray, ball, edgeIndex);
			}
		}
	}
	else
	{
		auto rayDyDX = (rayY - rayEndY) / (rayX - rayEndX);
		auto indexX = rayBoxX;
		auto indexY = rayBoxY;
		auto bresIndexX = rayBoxX + 1;
		auto bresIndexY = rayBoxY + 1;
		auto bresXAdd = rayY - rayDyDX * rayX;
		edgeIndex = TestGridBox(rayBoxX, rayBoxY, &distance, edge, ray, ball, 0);
		if (rayDirX == 1)
		{
			if (rayDirY == 1)
			{
				do
				{
					auto yCoord = bresIndexY * AdvanceY + Y;
					auto xCoord = (bresIndexX * AdvanceX + X) * rayDyDX + bresXAdd;
					if (xCoord >= yCoord)
					{
						if (xCoord == yCoord)
						{
							++indexX;
							++bresIndexX;
						}
						++indexY;
						++bresIndexY;
					}
					else
					{
						++indexX;
						++bresIndexX;
					}
					edgeIndex = TestGridBox(indexX, indexY, &distance, edge, ray, ball, edgeIndex);
				}
				while (indexX < rayEndBoxX || indexY < rayEndBoxY);
			}
			else
			{
				do
				{
					auto yCoord = indexY * AdvanceY + Y;
					auto xCoord = (bresIndexX * AdvanceX + X) * rayDyDX + bresXAdd;
					if (xCoord <= yCoord)
					{
						if (xCoord == yCoord)
						{
							++indexX;
							++bresIndexX;
						}
						--indexY;
					}
					else
					{
						++indexX;
						++bresIndexX;
					}
					edgeIndex = TestGridBox(indexX, indexY, &distance, edge, ray, ball, edgeIndex);
				}
				while (indexX < rayEndBoxX || indexY > rayEndBoxY);
			}
		}
		else
		{
			if (rayDirY == 1)
			{
				do
				{
					auto yCoord = bresIndexY * AdvanceY + Y;
					auto xCoord = (indexX * AdvanceX + X) * rayDyDX + bresXAdd;
					if (xCoord >= yCoord)
					{
						if (xCoord == yCoord)
							--indexX;
						++indexY;
						++bresIndexY;
					}
					else
					{
						--indexX;
					}
					edgeIndex = TestGridBox(indexX, indexY, &distance, edge, ray, ball, edgeIndex);
				}
				while (indexX > rayEndBoxX || indexY < rayEndBoxY);
			}
			else
			{
				do
				{
					auto yCoord = indexY * AdvanceY + Y;
					auto xCoord = (indexX * AdvanceX + X) * rayDyDX + bresXAdd;
					if (xCoord <= yCoord)
					{
						if (xCoord == yCoord)
							--indexX;
						--indexY;
					}
					else
					{
						--indexX;
					}
					edgeIndex = TestGridBox(indexX, indexY, &distance, edge, ray, ball, edgeIndex);
				}
				while (indexX > rayEndBoxX || indexY > rayEndBoxY);
			}
		}
	}


	for (auto edgePtr = EdgeArray; edgeIndex > 0; --edgeIndex, ++edgePtr)
	{
		(*edgePtr)->ProcessedFlag = 0;
	}

	return distance;
}
