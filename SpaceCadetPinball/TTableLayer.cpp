#include "pch.h"
#include "TTableLayer.h"

#include "loader.h"
#include "proj.h"
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
		PinballTable->TableAngleMult = tableAngleArr[0];
		PinballTable->TableAngle1 = tableAngleArr[1];
		PinballTable->TableAngle2 = tableAngleArr[2];
	}
	else
	{
		PinballTable->TableAngleMult = 25.0f;
		PinballTable->TableAngle1 = 0.5f;
		PinballTable->TableAngle2 = 1.570796f;
	}

	auto table3 = PinballTable;
	Angle1 = cos(table3->TableAngle2) * sin(table3->TableAngle1) * table3->TableAngleMult;
	Angle2 = sin(table3->TableAngle2) * sin(table3->TableAngle1) * table3->TableAngleMult;
	auto angleMultArr = loader::query_float_attribute(groupIndex, 0, 701);
	if (angleMultArr)
		AngleMult = *angleMultArr;
	else
		AngleMult = 0.2f;

	table->XOffset = bmp->XPosition;
	table->YOffset = bmp->YPosition;
	table->Width = bmp->Width;
	table->Height = bmp->Height;
	UnknownC7F = visual.Kicker.Unknown1F;
	UnknownC6F = 15.0f;

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

	Field.Unknown1 = -1;
	Field.Flag2Ptr = &UnknownBaseFlag2;
	Field.CollisionComp = this;
	TEdgeManager::edges_insert_square(Unknown2F, Unknown1F, Unknown4F, Unknown3F, nullptr,
	                                  &Field);
}

TTableLayer::~TTableLayer()
{
	if (edge_manager)
		delete edge_manager;
}
