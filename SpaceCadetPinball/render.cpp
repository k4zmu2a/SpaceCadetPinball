#include "pch.h"
#include "render.h"

#include "memory.h"

int render::blit = 0;
int render::many_dirty, render::many_sprites, render::many_balls;
render_sprite_type_struct **render::dirty_list = new render_sprite_type_struct*[1000], **render::sprite_list = new
	                          render_sprite_type_struct* [1000], **render::ball_list = new render_sprite_type_struct* [
	                          1000];
zmap_header_type* render::background_zmap;
int render::zmap_offset, render::zmap_offsetY;

void render::update()
{
}


void render::paint()
{
	/*render_paint_balls();
	gdrv_blat((int)&vscreen, xDest, yDest);
	render_unpaint_balls();*/
}


int render::sprite_modified(render_sprite_type_struct* sprite)
{
	int result = 0; // eax

	if (sprite->VisualType == VisualType::Ball)
		return result;
	result = many_dirty;
	if (many_dirty < 999)
		dirty_list[many_dirty++] = sprite;
	return result;
}

render_sprite_type_struct* render::create_sprite(VisualType visualType, gdrv_bitmap8* rootBmp8, zmap_header_type* zMap,
                                                 int xPosition, int yPosition, visual_rect* rect)
{
	render_sprite_type_struct* sprite = (render_sprite_type_struct*)memory::allocate(0x5Cu);
	render_sprite_type_struct* result = nullptr;
	if (!sprite)
		return result;
	sprite->YPosition = yPosition;
	sprite->RootBmp8 = rootBmp8;
	sprite->XPosition = xPosition;
	sprite->VisualType = visualType;
	sprite->Unknown6_0 = 0;
	sprite->Unknown17 = 0;
	sprite->Unknown18 = 0;
	if (rect)
	{
		sprite->Rect = *rect;
	}
	else
	{
		sprite->Rect.Width = -1;
		sprite->Rect.Height = -1;
		sprite->Rect.XPosition = 0;
		sprite->Rect.YPosition = 0;
	}
	if (rootBmp8)
	{
		sprite->Bmp8Width = rootBmp8->Width;
		sprite->Bmp8Height = rootBmp8->Height;
	}
	else
	{
		sprite->Bmp8Width = 0;
		sprite->Bmp8Height = 0;
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
	sprite->XPosition2 = sprite->XPosition;
	sprite->YPosition2 = sprite->YPosition;
	sprite->Bmp8Width2 = sprite->Bmp8Width;
	sprite->Bmp8Height2 = sprite->Bmp8Height;
	if (visualType == VisualType::Ball)
	{
		ball_list[many_balls++] = sprite;
	}
	else
	{
		sprite_list[many_sprites++] = sprite;
		sprite_modified(sprite);
	}
	result = sprite;
	return result;
}
