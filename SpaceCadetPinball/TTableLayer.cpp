#include "pch.h"
#include "TTableLayer.h"

#include "loader.h"
#include "proj.h"
#include "render.h"
#include "TLine.h"
#include "TPinballTable.h"

TEdgeManager* TTableLayer::edge_manager;

TTableLayer::TTableLayer(TPinballTable* table): TCollisionComponent(table, -1, false)
{
	visualStruct visual{};
	rectangle_type rect{};

	auto groupIndex = loader::query_handle("table");
	loader::query_visual(groupIndex, 0, &visual);

	auto projCenter = loader::query_float_attribute(groupIndex, 0, 700);
	proj::recenter(projCenter[0], projCenter[1]);
	render::set_background_zmap(visual.ZMap, 0, 0);

	auto bmp = visual.Bitmap;
	VisBmp = visual.Bitmap;
	rect.XPosition = 0;
	rect.YPosition = 0;
	rect.Width = bmp->Width;
	rect.Height = bmp->Height;
	render::create_sprite(VisualType::None, bmp, visual.ZMap, 0, 0, &rect);

	PinballTable->SoundIndex1 = visual.SoundIndex4;
	PinballTable->SoundIndex2 = visual.SoundIndex3;
	PinballTable->SoundIndex3 = visual.Kicker.SoundIndex;

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

	auto table3 = PinballTable;
	GraityDirX = cos(table3->GravityAnglY) * sin(table3->GravityAngleX) * table3->GravityDirVectMult;
	GraityDiY = sin(table3->GravityAnglY) * sin(table3->GravityAngleX) * table3->GravityDirVectMult;
	auto angleMultArr = loader::query_float_attribute(groupIndex, 0, 701);
	if (angleMultArr)
		GraityMult = *angleMultArr;
	else
		GraityMult = 0.2f;

	table->XOffset = bmp->XPosition;
	table->YOffset = bmp->YPosition;
	table->Width = bmp->Width;
	table->Height = bmp->Height;
	MaxCollisionSpeed = visual.Kicker.Unknown1F;
	CollisionMultiplier = 15.0f;

	auto visArrPtr = visual.FloatArr;
	Unknown1F = min(visArrPtr[0], min(visArrPtr[2], visArrPtr[4]));
	Unknown2F = min(visArrPtr[1], min(visArrPtr[3], visArrPtr[5]));
	Unknown3F = max(visArrPtr[0], max(visArrPtr[2], visArrPtr[4]));
	Unknown4F = max(visArrPtr[1], max(visArrPtr[3], visArrPtr[5]));
	auto a2 = Unknown4F - Unknown2F;
	auto a1 = Unknown3F - Unknown1F;
	edge_manager = new TEdgeManager(Unknown1F, Unknown2F, a1, a2);

	for (auto visFloatArrCount = visual.FloatArrCount; visFloatArrCount > 0; visFloatArrCount--)
	{
		auto line = new TLine(this,
		                      &UnknownBaseFlag2,
		                      visual.Flag,
		                      visArrPtr[2],
		                      visArrPtr[3],
		                      visArrPtr[0],
		                      visArrPtr[1]);
		if (line)
		{
			line->place_in_grid();
			EdgeList->Add(line);
		}

		visArrPtr += 2;
	}

	Field.Mask = -1;
	Field.Flag2Ptr = &UnknownBaseFlag2;
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
	vecDst->X = GraityDirX - (0.5f - static_cast<float>(rand()) * 0.00003051850947599719f + ball->Acceleration.X) *
		ball->Speed * GraityMult;
	vecDst->Y = GraityDiY - ball->Acceleration.Y * ball->Speed * GraityMult;
	return 1;
}

void TTableLayer::edges_insert_square(float y0, float x0, float y1, float x1, TEdgeSegment* edge,
                                      field_effect_type* field)
{
	float widthM = edge_manager->AdvanceX * 0.001f;
	float heightM = edge_manager->AdvanceY * 0.001f;
	float xMin = x0 - widthM;
	float xMax = x1 + widthM;
	float yMin = y0 - heightM;
	float yMax = y1 + heightM;

	int xMinBox = edge_manager->box_x(xMin);
	int yMinBox = edge_manager->box_y(yMin);
	int xMaxBox = edge_manager->box_x(xMax);
	int yMaxBox = edge_manager->box_y(yMax);

	float boxX = static_cast<float>(xMinBox) * edge_manager->AdvanceX + edge_manager->X;
	float boxY = static_cast<float>(yMinBox) * edge_manager->AdvanceY + edge_manager->Y;

	for (int indexX = xMinBox; indexX <= xMaxBox; ++indexX)
	{
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
