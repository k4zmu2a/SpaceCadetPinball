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

	/*Full tilt hack - GraityMult should be 0.2*/
	if (!pb::FullTiltMode && !pb::FullTiltDemoMode)
	{
		auto angleMultArr = loader::query_float_attribute(groupIndex, 0, 701);
		GraityMult = *angleMultArr;
	}
	else
		GraityMult = 0.2f;

	table->XOffset = bmp->XPosition;
	table->YOffset = bmp->YPosition;
	table->Width = bmp->Width;
	table->Height = bmp->Height;
	Threshold = visual.Kicker.Threshold;
	Boost = 15.0f;

	auto edgePoints = reinterpret_cast<vector2*>(visual.FloatArr);
	XMin = std::min(edgePoints[0].X, std::min(edgePoints[1].X, edgePoints[2].X));
	YMin = std::min(edgePoints[0].Y, std::min(edgePoints[1].Y, edgePoints[2].Y));
	XMax = std::max(edgePoints[0].X, std::max(edgePoints[1].X, edgePoints[2].X));
	YMax = std::max(edgePoints[0].Y, std::max(edgePoints[1].Y, edgePoints[2].Y));

	auto height = YMax - YMin;
	auto width = XMax - XMin;
	edge_manager = new TEdgeManager(XMin, YMin, width, height);

	for (auto i = 0; i < visual.FloatArrCount; i++)
	{
		auto line = new TLine(this,
		                      &ActiveFlag,
		                      visual.CollisionGroup,
		                      edgePoints[i + 1].X,
		                      edgePoints[i + 1].Y,
		                      edgePoints[i].X,
		                      edgePoints[i].Y);
		line->place_in_grid();
		EdgeList.push_back(line);
	}

	Field.CollisionGroup = -1;
	Field.ActiveFlag = &ActiveFlag;
	Field.CollisionComp = this;
	edges_insert_square(YMin, XMin, YMax, XMax, nullptr,
	                    &Field);
}

TTableLayer::~TTableLayer()
{
	if (edge_manager)
		delete edge_manager;
}

int TTableLayer::FieldEffect(TBall* ball, vector2* vecDst)
{
	vecDst->X = GraityDirX - (0.5f - RandFloat() + ball->Direction.X) *
		ball->Speed * GraityMult;
	vecDst->Y = GraityDirY - ball->Direction.Y * ball->Speed * GraityMult;
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

	float boxX = static_cast<float>(xMinBox) * edge_manager->AdvanceX + edge_manager->MinX;
	for (int indexX = xMinBox; indexX <= xMaxBox; ++indexX)
	{
		float boxY = static_cast<float>(yMinBox) * edge_manager->AdvanceY + edge_manager->MinY;
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
	vector2 vec1{};

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

	vec1.X = static_cast<float>(dirX) * edge_manager->AdvanceX + edge_manager->MinX;
	for (auto indexX = dirX; indexX <= xMaxBox; ++indexX)
	{
		vec1.Y = static_cast<float>(dirY) * edge_manager->AdvanceY + edge_manager->MinY;
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
					if (maths::ray_intersect_circle(ray, *circle) < 1000000000.0f)
						break;

					ray.Direction.X = -1.0;
					ray.Origin.X = ray.Origin.X + edge_manager->AdvanceX;
					if (maths::ray_intersect_circle(ray, *circle) < 1000000000.0f)
						break;

					ray.Direction.X = 0.0;
					ray.Direction.Y = 1.0;
					ray.MaxDistance = edge_manager->AdvanceY;
					if (maths::ray_intersect_circle(ray, *circle) < 1000000000.0f)
						break;

					ray.Direction.Y = -1.0;
					ray.Origin.Y = ray.Origin.Y + edge_manager->AdvanceY;
					if (maths::ray_intersect_circle(ray, *circle) < 1000000000.0f)
						break;

					ray.Direction.Y = 0.0;
					ray.Direction.X = -1.0;
					ray.MaxDistance = edge_manager->AdvanceX;
					if (maths::ray_intersect_circle(ray, *circle) < 1000000000.0f)
						break;

					ray.Direction.X = 1.0;
					ray.Origin.X = ray.Origin.X - edge_manager->AdvanceX;
					if (maths::ray_intersect_circle(ray, *circle) < 1000000000.0f)
						break;

					ray.Direction.X = 0.0;
					ray.Direction.Y = -1.0;
					ray.MaxDistance = edge_manager->AdvanceY;
					if (maths::ray_intersect_circle(ray, *circle) < 1000000000.0f)
						break;

					ray.Direction.Y = 1.0;
					ray.Origin.Y = ray.Origin.Y - edge_manager->AdvanceY;
					if (maths::ray_intersect_circle(ray, *circle) < 1000000000.0f)
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
