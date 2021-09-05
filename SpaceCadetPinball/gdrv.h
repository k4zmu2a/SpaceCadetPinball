#pragma once

enum class BitmapType : char
{
	None = 0,
	RawBitmap = 1,
	DibBitmap = 2,
	Spliced = 4,
};

struct gdrv_bitmap8
{
	char* BmpBufPtr1;
	int Width;
	int Height;
	int Stride;
	BitmapType BitmapType;
	int XPosition;
	int YPosition;
};

struct Rgba
{
	uint8_t peRed;
	uint8_t peGreen;
	uint8_t peBlue;
	uint8_t peFlags;
};
union ColorRgba
{
	uint32_t Color;
	Rgba rgba;
};
static_assert(sizeof(ColorRgba) == 4, "Wrong size of RGBA color");

struct LOGPALETTEx256 : LOGPALETTE
{
	PALETTEENTRY palPalEntry2[256 - 1];

	LOGPALETTEx256() : palPalEntry2{}
	{
		palVersion = 0x300;
		palNumEntries = 256;
	}
};


class gdrv
{
public:
	static SDL_Rect DestinationRect;

	static int init(int width, int height);
	static int uninit();
	static void get_focus();
	static int create_bitmap(gdrv_bitmap8* bmp, int width, int height);
	static int create_raw_bitmap(gdrv_bitmap8* bmp, int width, int height, int flag);
	static int create_spliced_bitmap(gdrv_bitmap8* bmp, int width, int height, int size);
	static int destroy_bitmap(gdrv_bitmap8* bmp);
	static int display_palette(ColorRgba* plt);
	static void start_blit_sequence();
	static void end_blit_sequence();
	static void blit(gdrv_bitmap8* bmp, int xSrc, int ySrc, int xDest, int yDest, int width, int height);
	static void blat(gdrv_bitmap8* bmp, int xDest, int yDest);
	static void fill_bitmap(gdrv_bitmap8* bmp, int width, int height, int xOff, int yOff, char fillChar);
	static void copy_bitmap(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff, gdrv_bitmap8* srcBmp,
	                        int srcXOff, int srcYOff);
	static void copy_bitmap_w_transparency(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff,
	                                       gdrv_bitmap8* srcBmp, int srcXOff, int srcYOff);
	static void grtext_draw_ttext_in_box(LPCSTR text, int xOff, int yOff, int width, int height, int a6);
	static void BlitScreen();
private:
	static SDL_Texture* vScreenTex;
	static char* vScreenPixels;
	static int vScreenWidth, vScreenHeight;
	static ColorRgba current_palette[256];

	static int StretchDIBitsScaled(int xSrc, int ySrc, int xDst, int yDst,
	                               int width, int height, gdrv_bitmap8* bmp);
};
