#pragma once
#include "gdrv.h"
#include "maths.h"
#include "zdrv.h"

enum class VisualTypes : char
{
	None = 0,
	Sprite = 1,
	Ball = 2
};

struct render_sprite_type_struct
{
	rectangle_type BmpRect;
	gdrv_bitmap8* Bmp;
	zmap_header_type* ZMap;
	char UnknownFlag;
	VisualTypes VisualType;
	int16_t Depth;
	rectangle_type DirtyRectPrev;
	int ZMapOffestY;
	int ZMapOffestX;
	rectangle_type DirtyRect;
	render_sprite_type_struct** SpriteArray;
	int SpriteCount;
	rectangle_type BoundingRect;
};


class render
{
public:
	static gdrv_bitmap8 vscreen, *background_bitmap;
	static SDL_Rect DestinationRect;

	static void init(gdrv_bitmap8* bmp, float zMin, float zScaler, int width, int height);
	static void uninit();
	static void update();
	static void sprite_modified(render_sprite_type_struct* sprite);
	static render_sprite_type_struct* create_sprite(VisualTypes visualType, gdrv_bitmap8* bmp,
	                                                zmap_header_type* zMap,
	                                                int xPosition, int yPosition, rectangle_type* rect);
	static void remove_sprite(render_sprite_type_struct* sprite);
	static void remove_ball(struct render_sprite_type_struct* ball);
	static void sprite_set(render_sprite_type_struct* sprite, gdrv_bitmap8* bmp, zmap_header_type* zMap, int xPos,
	                       int yPos);
	static void sprite_set_bitmap(render_sprite_type_struct* sprite, gdrv_bitmap8* bmp);
	static void set_background_zmap(struct zmap_header_type* zMap, int offsetX, int offsetY);
	static void ball_set(render_sprite_type_struct* sprite, gdrv_bitmap8* bmp, float depth, int xPos, int yPos);	
	static void shift(int offsetX, int offsetY);
	static void build_occlude_list();
	static void SpriteViewer(bool* show);
	static void PresentVScreen();
private:
	static int many_dirty, many_sprites, many_balls;
	static render_sprite_type_struct **dirty_list, **sprite_list, **ball_list;
	static zmap_header_type* background_zmap;
	static int zmap_offset, zmap_offsetY, offset_x, offset_y;
	static float zscaler, zmin, zmax;
	static rectangle_type vscreen_rect;
	static gdrv_bitmap8 ball_bitmap[20];
	static zmap_header_type zscreen;
	static SDL_Texture* vScreenTex;

	static void repaint(struct render_sprite_type_struct* sprite);
	static void paint_balls();
	static void unpaint_balls();
	static void BlitVScreen();
};
