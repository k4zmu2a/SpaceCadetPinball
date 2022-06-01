#include "pch.h"
#include "TPinballComponent.h"
#include "loader.h"
#include "proj.h"
#include "render.h"
#include "TPinballTable.h"
#include "TTableLayer.h"

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
	VisualPosNormX= -1.0f;
	VisualPosNormY = -1.0f;
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
				maths::enclosing_box(bmp1Rect, tmpRect, bmp1Rect);
			}

			RenderSprite = render::create_sprite(
				visualCount > 0 ? VisualTypes::Sprite : VisualTypes::None,
				rootBmp,
				zMap,
				rootBmp->XPosition - table->XOffset,
				rootBmp->YPosition - table->YOffset,
				&bmp1Rect);

			// Sound position = center of root visual, reverse-projected, normalized.
			auto& rect = RenderSprite->BmpRect;
			vector2i pos2D{ rect.XPosition + rect.Width / 2, rect.YPosition + rect.Height / 2 };
			auto pos3D = proj::ReverseXForm(pos2D);
			auto posNorm = TTableLayer::edge_manager->NormalizeBox(pos3D);
			VisualPosNormX = posNorm.X;
			VisualPosNormY = posNorm.Y;
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

vector2 TPinballComponent::get_coordinates()
{
	return {VisualPosNormX, VisualPosNormY};
}
