#include "pch.h"
#include "TPinballComponent.h"
#include "loader.h"
#include "objlist_class.h"
#include "render.h"
#include "TZmapList.h"
#include "TPinballTable.h"

TPinballComponent::TPinballComponent(TPinballTable* table, int groupIndex, bool loadVisuals)
{
	visualStruct visual{}; // [esp+Ch] [ebp-6Ch]

	MessageField = 0;
	UnknownBaseFlag1 = 0;
	UnknownBaseFlag2 = 0;
	PinballTable = table;
	RenderSprite = nullptr;
	ListBitmap = nullptr;
	ListZMap = nullptr;
	if (table)
		table->ListP1->Add(this);
	if (groupIndex >= 0)
		GroupName = loader::query_name(groupIndex);
	if (loadVisuals && groupIndex >= 0)
	{
		int visualCount = loader::query_visual_states(groupIndex);
		for (int index = 0; index < visualCount; ++index)
		{
			loader::query_visual(groupIndex, index, &visual);
			if (visual.Bitmap)
			{
				if (!ListBitmap)
					ListBitmap = new TZmapList(visualCount, 4);
				if (ListBitmap)
					ListBitmap->Add(visual.Bitmap);
			}
			if (visual.ZMap)
			{
				if (!ListZMap)
					ListZMap = new TZmapList(visualCount, 4);
				if (ListZMap)
					ListZMap->Add(visual.ZMap);
			}
		}
		zmap_header_type* zMap = nullptr;
		if (ListZMap)
			zMap = static_cast<zmap_header_type*>(ListZMap->Get(0));
		if (ListBitmap)
		{
			rectangle_type bmp1Rect{}, tmpRect{};
			auto rootBmp = static_cast<gdrv_bitmap8*>(ListBitmap->Get(0));
			bmp1Rect.XPosition = rootBmp->XPosition - table->XOffset;
			bmp1Rect.YPosition = rootBmp->YPosition - table->YOffset;
			bmp1Rect.Width = rootBmp->Width;
			bmp1Rect.Height = rootBmp->Height;
			for (int index = 1; index < ListBitmap->Count(); index++)
			{
				auto bmp = static_cast<gdrv_bitmap8*>(ListBitmap->Get(index));
				tmpRect.XPosition = bmp->XPosition - table->XOffset;
				tmpRect.YPosition = bmp->YPosition - table->YOffset;
				tmpRect.Width = bmp->Width;
				tmpRect.Height = bmp->Height;
				maths::enclosing_box(&bmp1Rect, &tmpRect, &bmp1Rect);
			}

			RenderSprite = render::create_sprite(
				visualCount > 0 ? VisualType::Sprite :VisualType::None,
				rootBmp,
				zMap,
				rootBmp->XPosition - table->XOffset,
				rootBmp->YPosition - table->YOffset,
				&bmp1Rect);
		}
	}
	GroupIndex = groupIndex;
}


TPinballComponent::~TPinballComponent()
{
	TPinballTable* table = PinballTable;
	if (table)
		table->ListP1->Delete(this);

	delete ListBitmap;
	delete ListZMap;
}


int TPinballComponent::Message(int message1, float message2)
{
	MessageField = message1;
	if (message1 == 1024)
		MessageField = 0;
	return 0;
}

void TPinballComponent::put_scoring(int score1, int score2)
{
}


int TPinballComponent::get_scoring(int score1)
{
	return 0;
}
