#include "pch.h"
#include "TPinballComponent.h"
#include "loader.h"
#include "render.h"
#include "TPinballTable.h"

TPinballComponent::TPinballComponent(TPinballTable* table, int groupIndex, bool loadVisuals)
{
	visualStruct visual{};

	MessageField = 0;
	UnusedBaseFlag = 0;
	ActiveFlag = 0;
	PinballTable = table;
	RenderSprite = nullptr;
	ListBitmap = nullptr;
	ListZMap = nullptr;
	GroupName = nullptr;
	Control = nullptr;
	if (table)
		table->ComponentList.push_back(this);
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
					ListBitmap = new std::vector<gdrv_bitmap8*>();
				if (ListBitmap)
					ListBitmap->push_back(visual.Bitmap);
			}
			if (visual.ZMap)
			{
				if (!ListZMap)
					ListZMap = new std::vector<zmap_header_type*>();
				if (ListZMap)
					ListZMap->push_back(visual.ZMap);
			}
		}

		auto zMap = ListZMap ? ListZMap->at(0) : nullptr;
		if (ListBitmap)
		{
			rectangle_type bmp1Rect{}, tmpRect{};
			auto rootBmp = ListBitmap->at(0);
			bmp1Rect.XPosition = rootBmp->XPosition - table->XOffset;
			bmp1Rect.YPosition = rootBmp->YPosition - table->YOffset;
			bmp1Rect.Width = rootBmp->Width;
			bmp1Rect.Height = rootBmp->Height;
			for (auto index = 1u; index < ListBitmap->size(); index++)
			{
				auto bmp = ListBitmap->at(index);
				tmpRect.XPosition = bmp->XPosition - table->XOffset;
				tmpRect.YPosition = bmp->YPosition - table->YOffset;
				tmpRect.Width = bmp->Width;
				tmpRect.Height = bmp->Height;
				maths::enclosing_box(&bmp1Rect, &tmpRect, &bmp1Rect);
			}

			RenderSprite = render::create_sprite(
				visualCount > 0 ? VisualTypes::Sprite : VisualTypes::None,
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
	if (PinballTable)
	{
		// ComponentList contains one reference to each component.
		auto& components = PinballTable->ComponentList;
		auto position = std::find(components.begin(), components.end(), this);
		if (position != components.end())
			components.erase(position);
	}

	delete ListBitmap;
	delete ListZMap;
}


int TPinballComponent::Message(int code, float value)
{
	MessageField = code;
	if (code == 1024)
		MessageField = 0;
	return 0;
}

void TPinballComponent::port_draw()
{
}

void TPinballComponent::put_scoring(int index, int score)
{
}

int TPinballComponent::get_scoring(int index)
{
	return 0;
}
