#pragma once
#include "gdrv.h"

struct zmap_header_type
{
	int Width;
	int Height;
	int Stride;
	unsigned Resolution;
	uint16_t* ZPtr1;
	SDL_Texture* Texture;
};

class zdrv
{
public:
	static int pad(int width);
	static int create_zmap(zmap_header_type* zmap, int width, int height, int stride = -1);
	static int destroy_zmap(zmap_header_type* zmap);
	static void fill(zmap_header_type* zmap, int width, int height, int xOff, int yOff, uint16_t fillWord);
	static void paint(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff,
	                  zmap_header_type* dstZMap, int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp, int srcBmpXOff,
	                  int srcBmpYOff, zmap_header_type* srcZMap, int srcZMapXOff, int srcZMapYOff);
	static void paint_flat(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff,
	                       zmap_header_type* zMap, int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp,
	                       int srcBmpXOff, int srcBmpYOff, uint16_t depth);
	static void CreatePreview(zmap_header_type& zMap);
	static void FlipZMapHorizontally(const zmap_header_type& zMap);
};
