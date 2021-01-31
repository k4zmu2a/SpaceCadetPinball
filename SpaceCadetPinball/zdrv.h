#pragma once
#include "gdrv.h"

struct zmap_header_type
{
	__int16 Width;
	__int16 Height;
	__int16 Stride;
	unsigned __int16* ZPtr1;
	unsigned __int16* ZPtr2;
	unsigned __int16 ZBuffer[1];
};

class zdrv
{
public:
	static int pad(int width);
	static int create_zmap(zmap_header_type* zmap, int width, int height);
	static int destroy_zmap(zmap_header_type* zmap);
	static void fill(zmap_header_type* zmap, int width, int height, int xOff, int yOff, unsigned __int16 fillChar);
	static void paint(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff,
	                  zmap_header_type* dstZMap, int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp, int srcBmpXOff,
	                  int srcBmpYOff, zmap_header_type* srcZMap, int srcZMapXOff, int srcZMapYOff);
	static void paint_flat(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff,
	                       zmap_header_type* zMap, int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp,
	                       int srcBmpXOff, int srcBmpYOff, unsigned __int16 depth);
	static void paint_spliced_bmp(int xPos, int yPos, gdrv_bitmap8* dstBmp, zmap_header_type* dstZmap,
	                          gdrv_bitmap8* srcBmp);
};
