#include "pch.h"
#include "render.h"

#include "GroupData.h"
#include "memory.h"
#include "options.h"
#include "pb.h"
#include "TPinballTable.h"
#include "winmain.h"

int render::many_dirty, render::many_sprites, render::many_balls;
render_sprite_type_struct **render::dirty_list, **render::sprite_list, **render::ball_list;
zmap_header_type* render::background_zmap;
int render::zmap_offset, render::zmap_offsetY, render::offset_x, render::offset_y;
float render::zscaler, render::zmin, render::zmax;
rectangle_type render::vscreen_rect;
gdrv_bitmap8 render::vscreen, *render::background_bitmap, render::ball_bitmap[20];
zmap_header_type render::zscreen;
SDL_Texture* render::vScreenTex = nullptr;
SDL_Rect render::DestinationRect{};

void render::init(gdrv_bitmap8* bmp, float zMin, float zScaler, int width, int height)
{
	++memory::critical_allocation;
	zscaler = zScaler;
	zmin = zMin;
	zmax = 4294967300.0f / zScaler + zMin;
	sprite_list = memory::allocate<render_sprite_type_struct*>(1000);
	dirty_list = memory::allocate<render_sprite_type_struct*>(1000);
	ball_list = memory::allocate<render_sprite_type_struct*>(20);
	gdrv::create_bitmap(&vscreen, width, height, width, false);
	zdrv::create_zmap(&zscreen, width, height);
	zdrv::fill(&zscreen, zscreen.Width, zscreen.Height, 0, 0, 0xFFFF);
	vscreen_rect.YPosition = 0;
	vscreen_rect.XPosition = 0;
	vscreen_rect.Width = width;
	vscreen_rect.Height = height;
	vscreen.YPosition = 0;
	vscreen.XPosition = 0;
	for (auto& ballBmp : ball_bitmap)
		gdrv::create_bitmap(&ballBmp, 64, 64, 64, false);

	background_bitmap = bmp;
	if (bmp)
		gdrv::copy_bitmap(&vscreen, width, height, 0, 0, bmp, 0, 0);
	else
		gdrv::fill_bitmap(&vscreen, vscreen.Width, vscreen.Height, 0, 0, 0);

	{
		UsingSdlHint hint{SDL_HINT_RENDER_SCALE_QUALITY, options::Options.LinearFiltering ? "linear" : "nearest"};
		vScreenTex = SDL_CreateTexture
		(
			winmain::Renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			width, height
		);
		SDL_SetTextureBlendMode(vScreenTex, SDL_BLENDMODE_NONE);
	}
	--memory::critical_allocation;
}

void render::uninit()
{
	gdrv::destroy_bitmap(&vscreen);
	zdrv::destroy_zmap(&zscreen);
	for (auto i = many_sprites - 1; i >= 0; --i)
		remove_sprite(sprite_list[i]);
	for (auto j = many_balls - 1; j >= 0; --j)
		remove_ball(ball_list[j]);
	for (auto& ballBmp : ball_bitmap)
		gdrv::destroy_bitmap(&ballBmp);
	memory::free(ball_list);
	memory::free(dirty_list);
	memory::free(sprite_list);
	many_sprites = 0;
	many_dirty = 0;
	many_balls = 0;
	SDL_DestroyTexture(vScreenTex);
}

void render::update()
{
	unpaint_balls();

	// Clip dirty sprites with vScreen, clear clipping (dirty) rectangles 
	for (int index = 0; index < many_dirty; ++index)
	{
		bool clearSprite = false;
		auto curSprite = dirty_list[index];
		switch (curSprite->VisualType)
		{
		case VisualTypes::Sprite:
			if (curSprite->DirtyRectPrev.Width > 0)
				maths::enclosing_box(&curSprite->DirtyRectPrev, &curSprite->BmpRect, &curSprite->DirtyRect);

			if (maths::rectangle_clip(&curSprite->DirtyRect, &vscreen_rect, &curSprite->DirtyRect))
				clearSprite = true;
			else
				curSprite->DirtyRect.Width = -1;
			break;
		case VisualTypes::None:
			if (maths::rectangle_clip(&curSprite->BmpRect, &vscreen_rect, &curSprite->DirtyRect))
				clearSprite = !curSprite->Bmp;
			else
				curSprite->DirtyRect.Width = -1;
			break;
		default: break;
		}

		if (clearSprite)
		{
			auto yPos = curSprite->DirtyRect.YPosition;
			auto width = curSprite->DirtyRect.Width;
			auto xPos = curSprite->DirtyRect.XPosition;
			auto height = curSprite->DirtyRect.Height;
			zdrv::fill(&zscreen, width, height, xPos, yPos, 0xFFFF);
			if (background_bitmap)
				gdrv::copy_bitmap(&vscreen, width, height, xPos, yPos, background_bitmap, xPos, yPos);
			else
				gdrv::fill_bitmap(&vscreen, width, height, xPos, yPos, 0);
		}
	}

	// Paint dirty rectangles of dirty sprites
	for (int index = 0; index < many_dirty; ++index)
	{
		auto sprite = dirty_list[index];
		if (sprite->DirtyRect.Width > 0 && (sprite->VisualType == VisualTypes::None || sprite->VisualType ==
			VisualTypes::Sprite))
			repaint(sprite);
	}

	paint_balls();

	// In the original, this used to blit dirty sprites and balls
	for (int index = 0; index < many_dirty; ++index)
	{
		auto sprite = dirty_list[index];
		sprite->DirtyRectPrev = sprite->DirtyRect;
		if (sprite->UnknownFlag != 0)
			remove_sprite(sprite);
	}

	many_dirty = 0;
}

void render::sprite_modified(render_sprite_type_struct* sprite)
{
	if (sprite->VisualType != VisualTypes::Ball && many_dirty < 999)
		dirty_list[many_dirty++] = sprite;
}

render_sprite_type_struct* render::create_sprite(VisualTypes visualType, gdrv_bitmap8* bmp, zmap_header_type* zMap,
                                                 int xPosition, int yPosition, rectangle_type* rect)
{
	auto sprite = memory::allocate<render_sprite_type_struct>();
	if (!sprite)
		return nullptr;
	sprite->BmpRect.YPosition = yPosition;
	sprite->BmpRect.XPosition = xPosition;
	sprite->Bmp = bmp;
	sprite->VisualType = visualType;
	sprite->UnknownFlag = 0;
	sprite->SpriteArray = nullptr;
	sprite->SpriteCount = 0;
	sprite->DirtyRect = rectangle_type{};
	if (rect)
	{
		sprite->BoundingRect = *rect;
	}
	else
	{
		sprite->BoundingRect.Width = -1;
		sprite->BoundingRect.Height = -1;
		sprite->BoundingRect.XPosition = 0;
		sprite->BoundingRect.YPosition = 0;
	}
	if (bmp)
	{
		sprite->BmpRect.Width = bmp->Width;
		sprite->BmpRect.Height = bmp->Height;
	}
	else
	{
		sprite->BmpRect.Width = 0;
		sprite->BmpRect.Height = 0;
	}
	sprite->ZMap = zMap;
	sprite->ZMapOffestX = 0;
	sprite->ZMapOffestY = 0;
	if (!zMap && visualType != VisualTypes::Ball)
	{
		sprite->ZMap = background_zmap;
		sprite->ZMapOffestY = xPosition - zmap_offset;
		sprite->ZMapOffestX = yPosition - zmap_offsetY;
	}
	sprite->DirtyRectPrev = sprite->BmpRect;
	if (visualType == VisualTypes::Ball)
	{
		ball_list[many_balls++] = sprite;
	}
	else
	{
		sprite_list[many_sprites++] = sprite;
		sprite_modified(sprite);
	}
	return sprite;
}


void render::remove_sprite(render_sprite_type_struct* sprite)
{
	int index = 0;
	if (many_sprites > 0)
	{
		while (sprite_list[index] != sprite)
		{
			if (++index >= many_sprites)
				return;
		}
		while (index < many_sprites)
		{
			sprite_list[index] = sprite_list[index + 1];
			++index;
		}
		many_sprites--;
		if (sprite->SpriteArray)
			memory::free(sprite->SpriteArray);
		memory::free(sprite);
	}
}

void render::remove_ball(struct render_sprite_type_struct* ball)
{
	int index = 0;
	if (many_balls > 0)
	{
		while (ball_list[index] != ball)
		{
			if (++index >= many_balls)
				return;
		}
		while (index < many_balls)
		{
			ball_list[index] = ball_list[index + 1];
			++index;
		}
		many_balls--;
		memory::free(ball);
	}
}

void render::sprite_set(render_sprite_type_struct* sprite, gdrv_bitmap8* bmp, zmap_header_type* zMap, int xPos,
                        int yPos)
{
	if (sprite)
	{
		sprite->BmpRect.XPosition = xPos;
		sprite->BmpRect.YPosition = yPos;
		sprite->Bmp = bmp;
		if (bmp)
		{
			sprite->BmpRect.Width = bmp->Width;
			sprite->BmpRect.Height = bmp->Height;
		}
		sprite->ZMap = zMap;
		sprite_modified(sprite);
	}
}

void render::sprite_set_bitmap(render_sprite_type_struct* sprite, gdrv_bitmap8* bmp)
{
	if (sprite && sprite->Bmp != bmp)
	{
		sprite->Bmp = bmp;
		if (bmp)
		{
			sprite->BmpRect.Width = bmp->Width;
			sprite->BmpRect.Height = bmp->Height;
		}
		sprite_modified(sprite);
	}
}

void render::set_background_zmap(struct zmap_header_type* zMap, int offsetX, int offsetY)
{
	background_zmap = zMap;
	zmap_offset = offsetX;
	zmap_offsetY = offsetY;
}

void render::ball_set(render_sprite_type_struct* sprite, gdrv_bitmap8* bmp, float depth, int xPos, int yPos)
{
	if (sprite)
	{
		sprite->Bmp = bmp;
		if (bmp)
		{
			sprite->BmpRect.XPosition = xPos;
			sprite->BmpRect.YPosition = yPos;
			sprite->BmpRect.Width = bmp->Width;
			sprite->BmpRect.Height = bmp->Height;
		}
		if (depth >= zmin)
		{
			float depth2 = (depth - zmin) * zscaler;
			if (depth2 <= zmax)
				sprite->Depth = static_cast<short>(depth2);
			else
				sprite->Depth = -1;
		}
		else
		{
			sprite->Depth = 0;
		}
	}
}

void render::repaint(struct render_sprite_type_struct* sprite)
{
	rectangle_type clipRect{};
	if (!sprite->SpriteArray)
		return;
	for (int index = 0; index < sprite->SpriteCount; index++)
	{
		auto refSprite = sprite->SpriteArray[index];
		if (!refSprite->UnknownFlag && refSprite->Bmp)
		{
			if (maths::rectangle_clip(&refSprite->BmpRect, &sprite->DirtyRect, &clipRect))
				zdrv::paint(
					clipRect.Width,
					clipRect.Height,
					&vscreen,
					clipRect.XPosition,
					clipRect.YPosition,
					&zscreen,
					clipRect.XPosition,
					clipRect.YPosition,
					refSprite->Bmp,
					clipRect.XPosition - refSprite->BmpRect.XPosition,
					clipRect.YPosition - refSprite->BmpRect.YPosition,
					refSprite->ZMap,
					clipRect.XPosition + refSprite->ZMapOffestY - refSprite->BmpRect.XPosition,
					clipRect.YPosition + refSprite->ZMapOffestX - refSprite->BmpRect.YPosition);
		}
	}
}


void render::paint_balls()
{
	// Sort ball sprites by depth
	for (auto i = 0; i < many_balls; i++)
	{
		for (auto j = i; j < many_balls / 2; ++j)
		{
			auto ballA = ball_list[j];
			auto ballB = ball_list[i];
			if (ballB->Depth > ballA->Depth)
			{
				ball_list[i] = ballA;
				ball_list[j] = ballB;
			}
		}
	}

	// For balls that clip vScreen: save original vScreen contents and paint ball bitmap.
	for (auto index = 0; index < many_balls; ++index)
	{
		auto ball = ball_list[index];
		auto dirty = &ball->DirtyRect;
		if (ball->Bmp && maths::rectangle_clip(&ball->BmpRect, &vscreen_rect, &ball->DirtyRect))
		{
			int xPos = dirty->XPosition;
			int yPos = dirty->YPosition;
			gdrv::copy_bitmap(&ball_bitmap[index], dirty->Width, dirty->Height, 0, 0, &vscreen, xPos, yPos);
			zdrv::paint_flat(
				dirty->Width,
				dirty->Height,
				&vscreen,
				xPos,
				yPos,
				&zscreen,
				xPos,
				yPos,
				ball->Bmp,
				xPos - ball->BmpRect.XPosition,
				yPos - ball->BmpRect.YPosition,
				ball->Depth);
		}
		else
		{
			dirty->Width = -1;
		}
	}
}

void render::unpaint_balls()
{
	// Restore portions of vScreen saved during previous paint_balls call.
	for (int index = many_balls - 1; index >= 0; index--)
	{
		auto curBall = ball_list[index];
		if (curBall->DirtyRect.Width > 0)
			gdrv::copy_bitmap(
				&vscreen,
				curBall->DirtyRect.Width,
				curBall->DirtyRect.Height,
				curBall->DirtyRect.XPosition,
				curBall->DirtyRect.YPosition,
				&ball_bitmap[index],
				0,
				0);

		curBall->DirtyRectPrev = curBall->DirtyRect;
	}
}

void render::shift(int offsetX, int offsetY)
{
	offset_x += offsetX;
	offset_y += offsetY;
}

void render::build_occlude_list()
{
	++memory::critical_allocation;
	render_sprite_type_struct** spriteArr = nullptr;
	for (int index = 0; index < many_sprites; ++index)
	{
		auto mainSprite = sprite_list[index];
		if (mainSprite->SpriteArray)
		{
			memory::free(mainSprite->SpriteArray);
			mainSprite->SpriteArray = nullptr;
			mainSprite->SpriteCount = 0;
		}

		if (!mainSprite->UnknownFlag && mainSprite->BoundingRect.Width != -1)
		{
			if (!spriteArr)
				spriteArr = memory::allocate<render_sprite_type_struct*>(1000);

			int occludeCount = 0;
			for (int i = 0; i < many_sprites; ++i)
			{
				auto refSprite = sprite_list[i];
				if (!refSprite->UnknownFlag
					&& refSprite->BoundingRect.Width != -1
					&& maths::rectangle_clip(&mainSprite->BoundingRect, &refSprite->BoundingRect, nullptr)
					&& spriteArr)
				{
					spriteArr[occludeCount++] = refSprite;
				}
			}

			if (!mainSprite->UnknownFlag && mainSprite->Bmp && occludeCount < 2)
				occludeCount = 0;
			if (occludeCount)
			{
				mainSprite->SpriteArray = memory::realloc(spriteArr, sizeof(void*) * occludeCount);
				mainSprite->SpriteCount = occludeCount;
				spriteArr = nullptr;
			}
		}
	}

	if (spriteArr)
		memory::free(spriteArr);

	--memory::critical_allocation;
}

void render::SpriteViewer(bool* show)
{
	static const char* BitmapTypes[] =
	{
		"None",
		"RawBitmap",
		"DibBitmap",
		"Spliced",
	};
	static float scale = 1.0f;
	auto uv_min = ImVec2(0.0f, 0.0f); // Top-left
	auto uv_max = ImVec2(1.0f, 1.0f); // Lower-right
	auto tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // No tint
	auto border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white

	if (ImGui::Begin("Sprite viewer", show, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			ImGui::SliderFloat("Sprite scale", &scale, 0.1f, 10.0f, "scale = %.3f");
			ImGui::EndMenuBar();
		}

		for (const auto group : pb::record_table->Groups)
		{
			bool emptyGroup = true;
			for (int i = 0; i <= 2; i++)
			{
				auto bmp = group->GetBitmap(i);
				if (bmp)
				{
					emptyGroup = false;
					break;
				}
			}
			if (emptyGroup)
				continue;

			ImGui::Text("Group: %d, name:%s", group->GroupId, group->GroupName.c_str());
			for (int i = 0; i <= 2; i++)
			{
				auto bmp = group->GetBitmap(i);
				if (!bmp)
					continue;

				auto type = BitmapTypes[static_cast<char>(bmp->BitmapType)];
				ImGui::Text("type:%s, size:%d, resolution: %dx%d, offset:%dx%d", type,
				            bmp->Resolution,
				            bmp->Width, bmp->Height, bmp->XPosition, bmp->YPosition);
			}

			for (int same = 0, i = 0; i <= 2; i++)
			{
				auto bmp = group->GetBitmap(i);
				if (!bmp)
					continue;

				gdrv::CreatePreview(*bmp);
				if (bmp->Texture)
				{
					if (!same)
						same = true;
					else
						ImGui::SameLine();

					ImGui::Image(bmp->Texture, ImVec2(bmp->Width * scale, bmp->Height * scale),
					             uv_min, uv_max, tint_col, border_col);
				}
			}

			for (int same = 0, i = 0; i <= 2; i++)
			{
				auto zMap = group->GetZMap(i);
				if (!zMap)
					continue;

				zdrv::CreatePreview(*zMap);
				if (zMap->Texture)
				{
					if (!same)
						same = true;
					else
						ImGui::SameLine();
					ImGui::Image(zMap->Texture, ImVec2(zMap->Width * scale, zMap->Height * scale),
					             uv_min, uv_max, tint_col, border_col);
				}
			}
		}
	}
	ImGui::End();
}

void render::BlitVScreen()
{
	int pitch = 0;
	ColorRgba* lockedPixels;
	SDL_LockTexture
	(
		vScreenTex,
		nullptr,
		reinterpret_cast<void**>(&lockedPixels),
		&pitch
	);
	assertm(pitch == vscreen.Width * sizeof(ColorRgba), "Padding on vScreen texture");

	if (offset_x == 0 && offset_y == 0)
	{
		// No offset - direct copy
		std::memcpy(lockedPixels, vscreen.BmpBufPtr1, vscreen.Width * vscreen.Height * sizeof(ColorRgba));
	}
	else
	{
		// Copy offset table and fixed side bar
		auto tableWidth = pb::MainTable->Width;
		auto scoreWidth = vscreen.Width - pb::MainTable->Width;
		auto tableStride = tableWidth * sizeof(ColorRgba);
		auto scoreStride = scoreWidth * sizeof(ColorRgba);
		auto srcScorePtr = &vscreen.BmpBufPtr1[tableWidth];

		auto xSrc = 0, ySrc = 0, xDst = offset_x, yDst = offset_y, height = vscreen.Height;

		// Negative dst == positive src offset
		if (xDst < 0)
		{
			xSrc -= xDst;
			xDst = 0;
		}
		if (yDst < 0)
		{
			ySrc -= yDst;
			yDst = 0;
		}

		if (xSrc)
		{
			tableStride -= xSrc * sizeof(ColorRgba);
		}
		if (xDst)
		{
			tableStride -= xDst * sizeof(ColorRgba);
			tableWidth -= xDst;
			scoreWidth += xDst;
		}
		if (ySrc)
			height -= ySrc;

		auto srcBmpPtr = &vscreen.BmpBufPtr1[vscreen.Width * ySrc + xSrc];
		auto dstPtr = &lockedPixels[vscreen.Width * yDst + xDst];
		for (int y = height; y > 0; --y)
		{
			std::memcpy(dstPtr, srcBmpPtr, tableStride);
			dstPtr += tableWidth;
			std::memcpy(dstPtr, srcScorePtr, scoreStride);
			dstPtr += scoreWidth;

			srcBmpPtr += vscreen.Stride;
			srcScorePtr += vscreen.Stride;
		}
	}


	SDL_UnlockTexture(vScreenTex);
}

void render::PresentVScreen()
{
	BlitVScreen();
	SDL_RenderCopy(winmain::Renderer, vScreenTex, nullptr, &DestinationRect);
}
