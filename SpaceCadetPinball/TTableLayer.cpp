#include "pch.h"
#include "TTableLayer.h"


#include "fullscrn.h"
#include "loader.h"
#include "pb.h"
#include "proj.h"
#include "render.h"
#include "TBall.h"
#include "TLine.h"
#include "TPinballTable.h"

TEdgeManager* TTableLayer::edge_manager;

TTableLayer::TTableLayer(TPinballTable* table): TCollisionComponent(table, -1, false)
{
	visualStruct visual{};
	rectangle_type rect{};

	auto groupIndex = loader::query_handle("table");
	loader::query_visual(groupIndex, 0, &visual);

	/*Full tilt: proj center first value is offset by resolution*/
	auto projCenter = loader::query_float_attribute(groupIndex, 0, 700 + fullscrn::GetResolution());
	proj::recenter(projCenter[0], projCenter[1]);
	render::set_background_zmap(visual.ZMap, 0, 0);

	auto bmp = visual.Bitmap;
	VisBmp = visual.Bitmap;
	rect.XPosition = 0;
	rect.YPosition = 0;
	rect.Width = bmp->Width;
	rect.Height = bmp->Height;
	render::create_sprite(VisualTypes::None, bmp, visual.ZMap, 0, 0, &rect);

	PinballTable->SoundIndex1 = visual.SoundIndex4;
	PinballTable->SoundIndex2 = visual.SoundIndex3;
	PinballTable->SoundIndex3 = visual.Kicker.HardHitSoundId;

	auto tableAngleArr = loader::query_float_attribute(groupIndex, 0, 305);
	if (tableAngleArr)
	{
		PinballTable->GravityDirVectMult = tableAngleArr[0];
		PinballTable->GravityAngleX = tableAngleArr[1];
		PinballTable->GravityAnglY = tableAngleArr[2];
	}
	else
	{
		PinballTable->GravityDirVectMult = 25.0f;
		PinballTable->GravityAngleX = 0.5f;
		PinballTable->GravityAnglY = 1.570796f;
	}

	GraityDirX = cos(PinballTable->GravityAnglY) * sin(PinballTable->GravityAngleX) * PinballTable->GravityDirVectMult;
	GraityDirY = sin(PinballTable->GravityAnglY) * sin(PinballTable->GravityAngleX) * PinballTable->GravityDirVectMult;
	auto angleMultArr = loader::query_float_attribute(groupIndex, 0, 701);

	/*Full tilt hack - GraityMult should be 0.2*/
	if (angleMultArr && !pb::FullTiltMode)
		GraityMult = *angleMultArr;
	else
		GraityMult = 0.2f;

	table->XOffset = bmp->XPosition;
	table->YOffset = bmp->YPosition;
	table->Width = bmp->Width;
	table->Height = bmp->Height;
	Threshold = visual.Kicker.Threshold;
	Boost = 15.0f;

	auto visArrPtr = visual.FloatArr;
	Unknown1F = std::min(visArrPtr[0], std::min(visArrPtr[2], visArrPtr[4]));
	Unknown2F = std::min(visArrPtr[1], std::min(visArrPtr[3], visArrPtr[5]));
	Unknown3F = std::max(visArrPtr[0], std::max(visArrPtr[2], visArrPtr[4]));
	Unknown4F = std::max(visArrPtr[1], std::max(visArrPtr[3], visArrPtr[5]));
	auto a2 = Unknown4F - Unknown2F;
	auto a1 = Unknown3F - Unknown1F;
	edge_manager = new TEdgeManager(Unknown1F, Unknown2F, a1, a2);

	for (auto visFloatArrCount = visual.FloatArrCount; visFloatArrCount > 0; visFloatArrCount--)
	{
		auto line = new TLine(this,
		                      &ActiveFlag,
		                      visual.CollisionGroup,
		                      visArrPtr[2],
		                      visArrPtr[3],
		                      visArrPtr[0],
		                      visArrPtr[1]);
		if (line)
		{
			line->place_in_grid();
			EdgeList.push_back(line);
		}

		visArrPtr += 2;
	}

	Field.Mask = -1;
	Field.Flag2Ptr = &ActiveFlag;
	Field.CollisionComp = this;
	edges_insert_square(Unknown2F, Unknown1F, Unknown4F, Unknown3F, nullptr,
	                    &Field);
}

TTableLayer::~TTableLayer()
{
	if (edge_manager)
		delete edge_manager;
}

int TTableLayer::FieldEffect(TBall* ball, vector_type* vecDst)
{
	vecDst->X = GraityDirX - (0.5f - RandFloat() + ball->Acceleration.X) *
		ball->Speed * GraityMult;
	vecDst->Y = GraityDirY - ball->Acceleration.Y * ball->Speed * GraityMult;
	return 1;
}

void TTableLayer::edges_insert_square(float y0, float x0, float y1, float x1, TEdgeSegment* edge,
                                      field_effect_type* field)
{
	float widthM = static_cast<float>(static_cast<int>(edge_manager->AdvanceX * 0.001f)); // Sic
	float heightM = static_cast<float>(static_cast<int>(edge_manager->AdvanceY * 0.001f));
	float xMin = x0 - widthM;
	float xMax = x1 + widthM;
	float yMin = y0 - heightM;
	float yMax = y1 + heightM;

	int xMinBox = edge_manager->box_x(xMin);
	int yMinBox = edge_manager->box_y(yMin);
	int xMaxBox = edge_manager->box_x(xMax);
	int yMaxBox = edge_manager->box_y(yMax);

	float boxX = static_cast<float>(xMinBox) * edge_manager->AdvanceX + edge_manager->X;
	for (int indexX = xMinBox; indexX <= xMaxBox; ++indexX)
	{
		float boxY = static_cast<float>(yMinBox) * edge_manager->AdvanceY + edge_manager->Y;
		for (int indexY = yMinBox; indexY <= yMaxBox; ++indexY)
		{
			if (xMax >= boxX && xMin <= boxX + edge_manager->AdvanceX &&
				yMax >= boxY && yMin <= boxY + edge_manager->AdvanceY)
			{
				if (edge)
				{
					edge_manager->add_edge_to_box(indexX, indexY, edge);
				}
				if (field)
				{
					edge_manager->add_field_to_box(indexX, indexY, field);
				}
			}
			boxY += edge_manager->AdvanceY;
		}
		boxX += edge_manager->AdvanceX;
	}
}

void TTableLayer::edges_insert_circle(circle_type* circle, TEdgeSegment* edge, field_effect_type* field)
{
	ray_type ray{};
	vector_type vec1{};

	auto radiusM = sqrt(circle->RadiusSq) + edge_manager->AdvanceX * 0.001f;
	auto radiusMSq = radiusM * radiusM;

	auto xMin = circle->Center.X - radiusM;
	auto yMin = circle->Center.Y - radiusM;
	auto xMax = radiusM + circle->Center.X;
	auto yMax = radiusM + circle->Center.Y;

	auto xMinBox = edge_manager->box_x(xMin);
	auto yMinBox = edge_manager->box_y(yMin);
	auto xMaxBox = edge_manager->box_x(xMax);
	auto yMaxBox = edge_manager->box_y(yMax);

	auto dirX = xMinBox - 1 <= 0 ? 0 : xMinBox - 1;
	auto dirY = yMinBox - 1 <= 0 ? 0 : yMinBox - 1;

	xMaxBox = edge_manager->increment_box_x(xMaxBox);
	yMaxBox = edge_manager->increment_box_y(yMaxBox);

	vec1.X = static_cast<float>(dirX) * edge_manager->AdvanceX + edge_manager->X;
	for (auto indexX = dirX; indexX <= xMaxBox; ++indexX)
	{
		vec1.Y = static_cast<float>(dirY) * edge_manager->AdvanceY + edge_manager->Y;
		for (int indexY = dirY; indexY <= yMaxBox; ++indexY)
		{
			auto vec1XAdv = vec1.X + edge_manager->AdvanceX;
			auto vec1YAdv = vec1.Y + edge_manager->AdvanceY;
			if (xMax >= vec1.X && xMin <= vec1XAdv &&
				yMax >= vec1.Y && yMin <= vec1YAdv)
			{
				bool collision = true;
				do
				{
					if (circle->Center.X <= vec1XAdv && circle->Center.X >= vec1.X &&
						circle->Center.Y <= vec1YAdv && circle->Center.Y >= vec1.Y)
						break;

					auto vec2 = vec1;
					if (maths::Distance_Squared(vec1, circle->Center) <= radiusMSq)
						break;

					vec2.X = vec2.X + edge_manager->AdvanceX;
					if (maths::Distance_Squared(vec2, circle->Center) <= radiusMSq)
						break;

					vec2.Y = vec2.Y + edge_manager->AdvanceY;
					if (maths::Distance_Squared(vec2, circle->Center) <= radiusMSq)
						break;

					vec2.X = vec2.X - edge_manager->AdvanceX;
					if (maths::Distance_Squared(vec2, circle->Center) <= radiusMSq)
						break;

					ray.Origin = vec1;
					ray.Direction.X = 1.0;
					ray.Direction.Y = 0.0;
					ray.MaxDistance = edge_manager->AdvanceX;
					if (maths::ray_intersect_circle(&ray, circle) < 1000000000.0f)
						break;

					ray.Direction.X = -1.0;
					ray.Origin.X = ray.Origin.X + edge_manager->AdvanceX;
					if (maths::ray_intersect_circle(&ray, circle) < 1000000000.0f)
						break;

					ray.Direction.X = 0.0;
					ray.Direction.Y = 1.0;
					ray.MaxDistance = edge_manager->AdvanceY;
					if (maths::ray_intersect_circle(&ray, circle) < 1000000000.0f)
						break;

					ray.Direction.Y = -1.0;
					ray.Origin.Y = ray.Origin.Y + edge_manager->AdvanceY;
					if (maths::ray_intersect_circle(&ray, circle) < 1000000000.0f)
						break;

					ray.Direction.Y = 0.0;
					ray.Direction.X = -1.0;
					ray.MaxDistance = edge_manager->AdvanceX;
					if (maths::ray_intersect_circle(&ray, circle) < 1000000000.0f)
						break;

					ray.Direction.X = 1.0;
					ray.Origin.X = ray.Origin.X - edge_manager->AdvanceX;
					if (maths::ray_intersect_circle(&ray, circle) < 1000000000.0f)
						break;

					ray.Direction.X = 0.0;
					ray.Direction.Y = -1.0;
					ray.MaxDistance = edge_manager->AdvanceY;
					if (maths::ray_intersect_circle(&ray, circle) < 1000000000.0f)
						break;

					ray.Direction.Y = 1.0;
					ray.Origin.Y = ray.Origin.Y - edge_manager->AdvanceY;
					if (maths::ray_intersect_circle(&ray, circle) < 1000000000.0f)
						break;

					collision = false;
				}
				while (false);
				if (collision)
				{
					if (edge)
					{
						edge_manager->add_edge_to_box(indexX, indexY, edge);
					}
					if (field)
					{
						edge_manager->add_field_to_box(indexX, indexY, field);
					}
				}
			}
			vec1.Y += edge_manager->AdvanceY;
		}
		vec1.X += edge_manager->AdvanceX;
	}
}
