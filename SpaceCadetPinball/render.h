#pragma once
#include "gdrv.h"
#include "maths.h"
#include "zdrv.h"

enum class VisualTypes : char
{
	Background = 0,
	Sprite = 1,
	Ball = 2
};

struct render_sprite
{
	rectangle_type BmpRect{};
	gdrv_bitmap8* Bmp;
	zmap_header_type* ZMap;
	bool DeleteFlag;
	VisualTypes VisualType;
	uint16_t Depth;
	rectangle_type DirtyRectPrev{};
	int ZMapOffestY;
	int ZMapOffestX;
	rectangle_type DirtyRect{};
	std::vector<render_sprite*>* OccludedSprites;
	rectangle_type BoundingRect{};
	bool DirtyFlag{};

	render_sprite(VisualTypes visualType, gdrv_bitmap8* bmp, zmap_header_type* zMap,
		int xPosition, int yPosition, rectangle_type* boundingRect);
	~render_sprite();
	void set(gdrv_bitmap8* bmp, zmap_header_type* zMap, int xPos, int yPos);
	void set_bitmap(gdrv_bitmap8* bmp);
	void ball_set(gdrv_bitmap8* bmp, float depth, int xPos, int yPos);
};


class render
{
public:
	static gdrv_bitmap8 *vscreen, *background_bitmap;
	static SDL_Rect DestinationRect;

	static void init(gdrv_bitmap8* bmp, int width, int height);
	static void uninit();
	static void recreate_screen_texture();
	static void update();
	static void AddSprite(render_sprite& sprite);
	static void RemoveSprite(render_sprite& sprite);
	static void set_background_zmap(zmap_header_type* zMap, int offsetX, int offsetY);
	static void shift(int offsetX, int offsetY);
	static void build_occlude_list();
	static void SpriteViewer(bool* show);
	static void PresentVScreen();
private:
	static std::vector<render_sprite*> sprite_list, ball_list;
	static zmap_header_type* background_zmap;
	static int zmap_offsetX, zmap_offsetY, offset_x, offset_y;
	static rectangle_type vscreen_rect;
	static gdrv_bitmap8 *ball_bitmap[20];
	static zmap_header_type* zscreen;

	static void repaint(const render_sprite& sprite);
	static void paint_balls();
	static void unpaint_balls();
};
