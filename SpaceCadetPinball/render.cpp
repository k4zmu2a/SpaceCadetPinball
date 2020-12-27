#include "pch.h"
#include "render.h"
#include "memory.h"

int render::blit = 0;
int render::many_dirty, render::many_sprites, render::many_balls;
render_sprite_type_struct **render::dirty_list, **render::sprite_list, **render::ball_list;
zmap_header_type* render::background_zmap;
int render::zmap_offset, render::zmap_offsetY, render::offset_x, render::offset_y;
float render::zscaler, render::zmin, render::zmax;
rectangle_type render::vscreen_rect;
gdrv_bitmap8 render::vscreen, *render::background_bitmap, render::ball_bitmap[20];
zmap_header_type render::zscreen;

void render::init(gdrv_bitmap8* bmp, float zMin, float zScaler, int width, int height)
{
	++memory::critical_allocation;
	zscaler = zScaler;
	zmin = zMin;
	zmax = 4294967300.0f / zScaler + zMin;
	sprite_list = (render_sprite_type_struct**)memory::allocate(0xFA0u);
	dirty_list = (render_sprite_type_struct**)memory::allocate(0xFA0u);
	ball_list = (render_sprite_type_struct**)memory::allocate(0x50u);
	gdrv::create_bitmap(&vscreen, width, height);
	zdrv::create_zmap(&zscreen, width, height);
	zdrv::fill(&zscreen, zscreen.Width, zscreen.Height, 0, 0, 0xFFFF);
	vscreen_rect.YPosition = 0;
	vscreen_rect.XPosition = 0;
	vscreen_rect.Width = width;
	vscreen_rect.Height = height;
	vscreen.YPosition = 0;
	vscreen.XPosition = 0;
	gdrv_bitmap8* ballBmp = ball_bitmap;
	while (ballBmp < &ball_bitmap[20])
	{
		gdrv::create_raw_bitmap(ballBmp, 64, 64, 1);
		++ballBmp;
	}
	background_bitmap = bmp;
	if (bmp)
		gdrv::copy_bitmap(&vscreen, width, height, 0, 0, bmp, 0, 0);
	else
		gdrv::fill_bitmap(&vscreen, vscreen.Width, vscreen.Height, 0, 0, 0);
	--memory::critical_allocation;
}

void render::uninit()
{
	gdrv::destroy_bitmap(&vscreen);
	zdrv::destroy_zmap(&zscreen);
	for (int i = 0; i < many_sprites; ++i)
		remove_sprite(sprite_list[i]);
	for (int j = 0; j < many_balls; ++j)
		remove_ball(ball_list[j]);
	memory::free(ball_list);
	memory::free(dirty_list);
	memory::free(sprite_list);
	many_sprites = 0;
	many_dirty = 0;
	many_balls = 0;
}

void render::update()
{
	rectangle_type overlapRect{};

	auto dirtyPtr = dirty_list;
	for (int index = 0; index < many_dirty; ++dirtyPtr, ++index)
	{
		auto curSprite = *dirtyPtr;
		if ((*dirtyPtr)->VisualType != VisualType::None)
		{
			if ((*dirtyPtr)->VisualType == VisualType::Sprite)
			{
				if (curSprite->BmpRectCopy.Width > 0)
					maths::enclosing_box(&curSprite->BmpRectCopy, &curSprite->BmpRect, &curSprite->DirtyRect);

				if (!maths::rectangle_clip(&curSprite->DirtyRect, &vscreen_rect, &curSprite->DirtyRect))
				{
					curSprite->DirtyRect.Width = -1;
					continue;
				}

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
		else
		{
			if (!maths::rectangle_clip(&curSprite->BmpRect, &vscreen_rect, &curSprite->DirtyRect))
			{
				curSprite->DirtyRect.Width = -1;
				continue;
			}
			if (!curSprite->Bmp)
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
	}

	dirtyPtr = dirty_list;
	for (int index = 0; index < many_dirty; ++index)
	{
		auto sprite = *dirtyPtr;
		if ((*dirtyPtr)->DirtyRect.Width > 0 && (sprite->VisualType == VisualType::None || sprite->VisualType ==
			VisualType::Sprite))
			repaint(*dirtyPtr);
		++dirtyPtr;
	}

	paint_balls();
	if (blit)
	{
		gdrv::start_blit_sequence();

		auto xPos = vscreen.XPosition + offset_x;
		auto yPos = vscreen.YPosition + offset_y;
		dirtyPtr = dirty_list;
		for (int index = 0; index < many_dirty; ++dirtyPtr, ++index)
		{
			auto sprite = *dirtyPtr;
			auto dirtyRect = &(*dirtyPtr)->DirtyRect;
			auto width2 = (*dirtyPtr)->DirtyRect.Width;
			if (width2 > 0)
				gdrv::blit_sequence(
					&vscreen,
					dirtyRect->XPosition,
					dirtyRect->YPosition,
					dirtyRect->XPosition + xPos,
					dirtyRect->YPosition + yPos,
					width2,
					dirtyRect->Height);

			auto rect = &sprite->BmpRectCopy;
			rect->XPosition = dirtyRect->XPosition;
			rect->YPosition = dirtyRect->YPosition;
			rect->Width = dirtyRect->Width;
			rect->Height = dirtyRect->Height;

			if (sprite->UnknownFlag != 0)
				remove_sprite(sprite);
		}

		dirtyPtr = ball_list;
		for (int index = 0; index < many_balls; ++dirtyPtr, ++index)
		{
			auto rectCopy = &(*dirtyPtr)->BmpRectCopy;
			auto dirtyRect = &(*dirtyPtr)->DirtyRect;
			if (maths::overlapping_box(dirtyRect, rectCopy, &overlapRect) && dirtyRect->Width > 0)
			{
				if (overlapRect.Width > 0)
					gdrv::blit_sequence(
						&vscreen,
						overlapRect.XPosition,
						overlapRect.YPosition,
						overlapRect.XPosition + xPos,
						overlapRect.YPosition + yPos,
						overlapRect.Width,
						overlapRect.Height);
			}
			else
			{
				if (dirtyRect->Width > 0)
					gdrv::blit_sequence(
						&vscreen,
						dirtyRect->XPosition,
						dirtyRect->YPosition,
						dirtyRect->XPosition + xPos,
						dirtyRect->YPosition + yPos,
						dirtyRect->Width,
						dirtyRect->Height);
				if (rectCopy->Width > 0)
					gdrv::blit_sequence(
						&vscreen,
						rectCopy->XPosition,
						rectCopy->YPosition,
						rectCopy->XPosition + xPos,
						rectCopy->YPosition + yPos,
						rectCopy->Width,
						rectCopy->Height);
			}
		}

		gdrv::end_blit_sequence();
	}

	many_dirty = 0;
	unpaint_balls();
}


void render::paint()
{
	paint_balls();
	gdrv::blat(&vscreen, vscreen.XPosition, vscreen.YPosition);
	unpaint_balls();
}

void render::sprite_modified(render_sprite_type_struct* sprite)
{
	if (sprite->VisualType != VisualType::Ball && many_dirty < 999)
		dirty_list[many_dirty++] = sprite;
}

render_sprite_type_struct* render::create_sprite(VisualType visualType, gdrv_bitmap8* bmp, zmap_header_type* zMap,
                                                 int xPosition, int yPosition, rectangle_type* rect)
{
	auto sprite = (render_sprite_type_struct*)memory::allocate(sizeof(render_sprite_type_struct));
	if (!sprite)
		return nullptr;
	sprite->BmpRect.YPosition = yPosition;
	sprite->BmpRect.XPosition = xPosition;
	sprite->Bmp = bmp;
	sprite->VisualType = visualType;
	sprite->UnknownFlag = 0;
	sprite->SpriteArray = nullptr;
	sprite->SpriteCount = 0;
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
	if (!zMap && visualType != VisualType::Ball)
	{
		sprite->ZMap = background_zmap;
		sprite->ZMapOffestY = xPosition - zmap_offset;
		sprite->ZMapOffestX = yPosition - zmap_offsetY;
	}
	sprite->BmpRectCopy = sprite->BmpRect;
	if (visualType == VisualType::Ball)
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
	int spriteCount = many_sprites;
	int index = 0;
	if (many_sprites > 0)
	{
		while (sprite_list[index] != sprite)
		{
			if (++index >= many_sprites)
				return;
		}
		while (index < spriteCount)
		{
			sprite_list[index] = sprite_list[index + 1];
			spriteCount = many_sprites;
			++index;
		}
		many_sprites = spriteCount - 1;
		if (sprite->SpriteArray)
			memory::free(sprite->SpriteArray);
		memory::free(sprite);
	}
}

void render::remove_ball(struct render_sprite_type_struct* ball)
{
	int ballCount = many_balls;
	int index = 0;
	if (many_balls > 0)
	{
		while (ball_list[index] != ball)
		{
			if (++index >= many_balls)
				return;
		}
		while (index < ballCount)
		{
			ball_list[index] = ball_list[index + 1];
			ballCount = many_balls;
			++index;
		}
		many_balls = ballCount - 1;
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
		render_sprite_type_struct* curSprite = sprite->SpriteArray[index];
		if (!curSprite->UnknownFlag && curSprite->Bmp)
		{
			if (maths::rectangle_clip(&curSprite->BmpRect, &sprite->DirtyRect, &clipRect))
				zdrv::paint(
					clipRect.Width,
					clipRect.Height,
					&vscreen,
					clipRect.XPosition,
					clipRect.YPosition,
					&zscreen,
					clipRect.XPosition,
					clipRect.YPosition,
					curSprite->Bmp,
					clipRect.XPosition - curSprite->BmpRect.XPosition,
					clipRect.YPosition - curSprite->BmpRect.YPosition,
					curSprite->ZMap,
					clipRect.XPosition + curSprite->ZMapOffestY - curSprite->BmpRect.XPosition,
					clipRect.YPosition + curSprite->ZMapOffestX - curSprite->BmpRect.YPosition);
		}
	}
}


void render::paint_balls()
{
	int ballCount = many_balls;
	if (many_balls > 1)
	{
		for (int index = 0; index < ballCount; index++)
		{
			for (int i = index; i < ballCount / 2; ++i)
			{
				auto curBall = ball_list[i];
				auto firstBallPtr = &ball_list[index];
				if ((*firstBallPtr)->Depth > curBall->Depth)
				{
					auto firstBall = *firstBallPtr;
					*firstBallPtr = curBall;
					ball_list[i] = firstBall;
				}
			}
		}
	}

	auto ballPtr = ball_list;
	auto ballBmpPtr = ball_bitmap;
	for (int index2 = 0; index2 < many_balls; ++index2)
	{
		struct render_sprite_type_struct* sprite = *ballPtr;
		rectangle_type* rect2 = &(*ballPtr)->DirtyRect;
		if ((*ballPtr)->Bmp && maths::rectangle_clip(&sprite->BmpRect, &vscreen_rect, &(*ballPtr)->DirtyRect))
		{
			int xPos = rect2->XPosition;
			int yPos = rect2->YPosition;
			gdrv::copy_bitmap(ballBmpPtr, rect2->Width, rect2->Height, 0, 0, &vscreen, xPos, yPos);
			zdrv::paint_flat(
				rect2->Width,
				rect2->Height,
				&vscreen,
				xPos,
				yPos,
				&zscreen,
				xPos,
				yPos,
				sprite->Bmp,
				xPos - sprite->BmpRect.XPosition,
				yPos - sprite->BmpRect.YPosition,
				sprite->Depth);
		}
		else
		{
			rect2->Width = -1;
		}

		++ballBmpPtr;
		++ballPtr;
	}
}

void render::unpaint_balls()
{
	auto ballPtr = &ball_list[many_balls - 1];
	if (many_balls - 1 >= 0)
	{
		gdrv_bitmap8* bitmapPtr = &ball_bitmap[many_balls - 1];
		for (int index = many_balls; index > 0; index--)
		{
			struct render_sprite_type_struct* curBall = *ballPtr;
			rectangle_type* rect2 = &(*ballPtr)->DirtyRect;
			int width = (*ballPtr)->DirtyRect.Width;
			if (width > 0)
				gdrv::copy_bitmap(
					&vscreen,
					width,
					(*ballPtr)->DirtyRect.Height,
					(*ballPtr)->DirtyRect.XPosition,
					(*ballPtr)->DirtyRect.YPosition,
					bitmapPtr,
					0,
					0);

			rectangle_type* rectCopy = &curBall->BmpRectCopy;
			rectCopy->XPosition = rect2->XPosition;
			rectCopy->YPosition = rect2->YPosition;
			rectCopy->Width = rect2->Width;
			rectCopy->Height = rect2->Height;

			--ballPtr;
			--bitmapPtr;
		}
	}
}

void render::shift(int offsetX, int offsetY, int xSrc, int ySrc, int DestWidth, int DestHeight)
{
	offset_x += offsetX;
	offset_y += offsetY;
	paint_balls();
	gdrv::blit(
		&vscreen,
		xSrc,
		ySrc,
		xSrc + offset_x + vscreen.XPosition,
		ySrc + offset_y + vscreen.YPosition,
		DestWidth,
		DestHeight);
	unpaint_balls();
}

void render::build_occlude_list()
{
	++memory::critical_allocation;
	render_sprite_type_struct** spriteArr = nullptr;
	auto spritePtr1 = sprite_list;
	for (int index = 0; index < many_sprites; ++index, ++spritePtr1)
	{
		auto curSprite = *spritePtr1;
		if ((*spritePtr1)->SpriteArray)
		{
			memory::free((*spritePtr1)->SpriteArray);
			curSprite->SpriteArray = nullptr;
			curSprite->SpriteCount = 0;
		}
		if (!curSprite->UnknownFlag && curSprite->BoundingRect.Width != -1)
		{
			if (!spriteArr)
				spriteArr = reinterpret_cast<render_sprite_type_struct**>(memory::allocate(0xFA0u));
			int occludeCount = 0;
			auto spritePtr2 = sprite_list;
			for (int i = 0; i < many_sprites; ++i, ++spritePtr2)
			{
				auto sprite = *spritePtr2;
				if (!sprite->UnknownFlag
					&& sprite->BoundingRect.Width != -1
					&& maths::rectangle_clip(&curSprite->BoundingRect, &sprite->BoundingRect, nullptr)
					&& spriteArr)
				{
					spriteArr[occludeCount++] = sprite;
				}
			}
			if (!curSprite->UnknownFlag && curSprite->Bmp && occludeCount < 2)
				occludeCount = 0;
			if (occludeCount)
			{
				curSprite->SpriteArray = reinterpret_cast<render_sprite_type_struct**>(memory::realloc(
					spriteArr, 4 * occludeCount));
				curSprite->SpriteCount = occludeCount;
				spriteArr = nullptr;
			}
		}
	}

	if (spriteArr)
		memory::free(spriteArr);

	--memory::critical_allocation;
}
