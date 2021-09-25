#pragma once

enum class BitmapTypes : uint8_t
{
	None = 0,
	RawBitmap = 1,
	DibBitmap = 2,
	Spliced = 3,
};


struct Rgba
{
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
};

union ColorRgba
{
	ColorRgba() = default;

	explicit ColorRgba(uint32_t color)
		: Color(color)
	{
	}

	explicit ColorRgba(Rgba rgba)
		: rgba(rgba)
	{
	}

	uint32_t Color;
	Rgba rgba;
};

static_assert(sizeof(ColorRgba) == 4, "Wrong size of RGBA color");

struct gdrv_bitmap8
{
	ColorRgba* BmpBufPtr1;
	char* IndexedBmpPtr;
	int Width;
	int Height;
	int Stride;
	int IndexedStride;
	BitmapTypes BitmapType;
	int XPosition;
	int YPosition;
	unsigned Resolution;
	//ColorRgba* RgbaBuffer;
	SDL_Texture* Texture;
};


class gdrv
{
public:
	static int create_bitmap(gdrv_bitmap8* bmp, int width, int height, int stride = -1, bool indexed = true);
	static int create_bitmap(gdrv_bitmap8& bmp, const struct dat8BitBmpHeader& header);
	static int destroy_bitmap(gdrv_bitmap8* bmp);
	static int display_palette(ColorRgba* plt);
	static void fill_bitmap(gdrv_bitmap8* bmp, int width, int height, int xOff, int yOff, uint8_t fillChar);
	static void copy_bitmap(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff, gdrv_bitmap8* srcBmp,
	                        int srcXOff, int srcYOff);
	static void copy_bitmap_w_transparency(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff,
	                                       gdrv_bitmap8* srcBmp, int srcXOff, int srcYOff);
	static void grtext_draw_ttext_in_box(LPCSTR text, int xOff, int yOff, int width, int height, int a6);
	static void ApplyPalette(gdrv_bitmap8& bmp);
	static void CreatePreview(gdrv_bitmap8& bmp);
private:
	static ColorRgba current_palette[256];
};
