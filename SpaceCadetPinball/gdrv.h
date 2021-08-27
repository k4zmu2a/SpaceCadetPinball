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
	BITMAPINFO* Dib;
	char* BmpBufPtr2;
	char* BmpBufPtr1;
	int Width;
	int Height;
	int Stride;
	BitmapType BitmapType;
	int Color6;
	int XPosition;
	int YPosition;
};

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
	static HPALETTE palette_handle;
	static int sequence_handle;
	static HDC sequence_hdc;
	static int use_wing;

	static int init(SDL_Renderer* renderer, int width, int height);
	static int uninit();
	static void get_focus();
	static BITMAPINFO* DibCreate(int16_t bpp, int width, int height);
	static void DibSetUsage(BITMAPINFO* dib, HPALETTE hpal, int someFlag);
	static int create_bitmap_dib(gdrv_bitmap8* bmp, int width, int height);
	static int create_bitmap(gdrv_bitmap8* bmp, int width, int height);
	static int create_raw_bitmap(gdrv_bitmap8* bmp, int width, int height, int flag);
	static int create_spliced_bitmap(gdrv_bitmap8* bmp, int width, int height, int size);
	static int destroy_bitmap(gdrv_bitmap8* bmp);
	static int display_palette(PALETTEENTRY* plt);
	static void start_blit_sequence();
	static void blit_sequence(gdrv_bitmap8* bmp, int xSrc, int ySrcOff, int xDest, int yDest, int DestWidth,
	                          int DestHeight);
	static void end_blit_sequence();
	static void blit(gdrv_bitmap8* bmp, int xSrc, int ySrcOff, int xDest, int yDest, int DestWidth, int DestHeight);
	static void blat(gdrv_bitmap8* bmp, int xDest, int yDest);
	static void fill_bitmap(gdrv_bitmap8* bmp, int width, int height, int xOff, int yOff, char fillChar);
	static void copy_bitmap(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff, gdrv_bitmap8* srcBmp,
	                        int srcXOff, int srcYOff);
	static void copy_bitmap_w_transparency(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff,
	                                       gdrv_bitmap8* srcBmp, int srcXOff, int srcYOff);
	static void grtext_draw_ttext_in_box(LPCSTR text, int xOff, int yOff, int width, int height, int a6);
private:
	/*COLORONCOLOR or HALFTONE*/
	static const int stretchMode = COLORONCOLOR;
	static SDL_Renderer* renderer;
	static int grtext_blue;
	static int grtext_green;
	static int grtext_red;

	static int StretchDIBitsScaled(HDC hdc, int xDest, int yDest, int DestWidth, int DestHeight, int xSrc, int ySrc,
	                               int SrcWidth, int SrcHeight, gdrv_bitmap8* bmp, UINT iUsage,
	                               DWORD rop);
};
