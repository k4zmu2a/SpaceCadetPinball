#pragma once
#include "gdrv.h"
#include "maths.h"
#include "zdrv.h"

enum class VisualType : char
{
	None = 0,
	Sprite = 1,
	Ball = 2
};

struct __declspec(align(4)) render_sprite_type_struct
{
	int XPosition;
	int YPosition;
	int Bmp8Width;
	int Bmp8Height;
	gdrv_bitmap8* RootBmp8;
	zmap_header_type* ZMap;
	char Unknown6_0;
	VisualType VisualType;
	short Depth;
	int XPosition2;
	int YPosition2;
	int Bmp8Width2;
	int Bmp8Height2;
	int ZMapOffestY;
	int ZMapOffestX;
	int Unknown13;
	int Unknown14;
	int Unknown15;
	int Unknown16;
	int Unknown17;
	int Unknown18;
	visual_rect Rect;
};

static_assert(sizeof(render_sprite_type_struct) == 0x5c, "Wrong size render_sprite_type_struct");

class render
{
public:
	static int blit;
	static int many_dirty, many_sprites, many_balls;
	static render_sprite_type_struct **dirty_list, **sprite_list, **ball_list;
	static zmap_header_type* background_zmap;
	static int zmap_offset, zmap_offsetY;

	static void update();
	static void paint();
	static int sprite_modified(render_sprite_type_struct* sprite);
	static render_sprite_type_struct* create_sprite(VisualType visualType, gdrv_bitmap8* rootBmp8,
	                                                zmap_header_type* zMap,
	                                                int xPosition, int yPosition, visual_rect* rect);
};
