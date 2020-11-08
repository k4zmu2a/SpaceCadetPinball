#pragma once

union tagPALETTEENTRY2
{
	unsigned __int32 PltInt;
	tagPALETTEENTRY Plt;
};

struct gdrv_dib_palette
{
	tagPALETTEENTRY2 Color0;
	tagPALETTEENTRY2 Color1;
	tagPALETTEENTRY2 Color2;
	tagPALETTEENTRY2 Color3;
	tagPALETTEENTRY2 Color4;
	tagPALETTEENTRY2 Color5;
	tagPALETTEENTRY2 Color6;
	tagPALETTEENTRY2 Color7;
	tagPALETTEENTRY2 Color8;
	tagPALETTEENTRY2 Color9;
	tagPALETTEENTRY2 Color10;
	tagPALETTEENTRY2 Color11;
	tagPALETTEENTRY2 Color12;
	tagPALETTEENTRY2 Color13;
	tagPALETTEENTRY2 Color14;
	tagPALETTEENTRY2 Color15;
};

struct __declspec(align(4)) gdrv_dib
{
	int PaletteOffset;
	int Width;
	int Height;
	__int16 Unknown3_1;
	unsigned __int16 Bpp;
	int Unknown4;
	int BufferSize;
	int Unknown6;
	int Unknown7;
	int NumberOfColors;
	int Unknown9;
	gdrv_dib_palette Palette0;
	gdrv_dib_palette Palette1;
	gdrv_dib_palette Palette2;
	gdrv_dib_palette Palette3;
	gdrv_dib_palette Palette4;
	gdrv_dib_palette Palette5;
	gdrv_dib_palette Palette6;
	gdrv_dib_palette Palette7;
	gdrv_dib_palette Palette8;
	gdrv_dib_palette Palette9;
	gdrv_dib_palette Palette10;
	gdrv_dib_palette Palette11;
	gdrv_dib_palette Palette12;
	gdrv_dib_palette Palette13;
	gdrv_dib_palette Palette14;
	gdrv_dib_palette Palette15;
	char BmpBuffer[1];
};

#pragma pack(push, 1)
struct __declspec(align(1)) gdrv_bitmap8
{
	gdrv_dib* Dib;
	char* BmpBufPtr2;
	char* BmpBufPtr1;
	int Width;
	int Height;
	int Stride;
	char SomeByte;
	int Color6;
	int XPosition;
	int YPosition;
};
#pragma pack(pop)

static_assert(sizeof(tagPALETTEENTRY2) == 4, "Wrong size of tagPALETTEENTRY2");
static_assert(sizeof(gdrv_dib_palette) == 4 * 16, "Wrong size of gdrv_dib_palette");
static_assert(sizeof(gdrv_dib) == (10 * 4) + sizeof(gdrv_dib_palette) * 16 + 4, "Wrong size of gdrv_dib");
static_assert(sizeof(gdrv_bitmap8) == 37, "Wrong size of gdrv_bitmap8");

class gdrv
{
public:
	static HPALETTE palette_handle;
	static void get_focus();
	static gdrv_dib* DibCreate(__int16 bpp, int width, int height);
	static void DibSetUsage(gdrv_dib* dib, HPALETTE hpal, int someFlag);
	static int create_bitmap_dib(gdrv_bitmap8* bmp, int width, int height);
	static int create_bitmap(gdrv_bitmap8* bmp, int width, int height);
	static int create_raw_bitmap(gdrv_bitmap8* bmp, int width, int height, int flag);
private:
};
