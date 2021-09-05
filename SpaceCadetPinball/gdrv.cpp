#include "pch.h"
#include "gdrv.h"

#include "fullscrn.h"
#include "memory.h"
#include "render.h"
#include "winmain.h"

SDL_Texture* gdrv::vScreenTex = nullptr;
char* gdrv::vScreenPixels = nullptr;
int gdrv::vScreenWidth, gdrv::vScreenHeight;
ColorRgba gdrv::current_palette[256]{};
SDL_Rect gdrv::DestinationRect{};

int gdrv::init(int width, int height)
{
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	vScreenTex = SDL_CreateTexture
	(
		winmain::Renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		width, height
	);
	vScreenPixels = memory::allocate(width * height * 4);
	vScreenWidth = width;
	vScreenHeight = height;

	return 0;
}

int gdrv::uninit()
{
	SDL_DestroyTexture(vScreenTex);
	memory::free(vScreenPixels);
	return 0;
}

void gdrv::get_focus()
{
}

int gdrv::create_bitmap(gdrv_bitmap8* bmp, int width, int height)
{
	bmp->Width = width;
	bmp->Stride = width;
	if (width % 4)
		bmp->Stride = 4 - width % 4 + width;

	bmp->Height = height;
	bmp->BitmapType = BitmapType::DibBitmap;
	bmp->BmpBufPtr1 = memory::allocate(bmp->Height * bmp->Stride);
	return 0;
}

int gdrv::create_raw_bitmap(gdrv_bitmap8* bmp, int width, int height, int flag)
{
	bmp->Width = width;
	bmp->Stride = width;
	if (flag && width % 4)
		bmp->Stride = width - width % 4 + 4;
	unsigned int sizeInBytes = height * bmp->Stride;
	bmp->Height = height;
	bmp->BitmapType = BitmapType::RawBitmap;
	char* buf = memory::allocate(sizeInBytes);
	bmp->BmpBufPtr1 = buf;
	if (!buf)
		return -1;
	return 0;
}

int gdrv::create_spliced_bitmap(gdrv_bitmap8* bmp, int width, int height, int size)
{
	bmp->Width = width;
	bmp->Stride = width;
	bmp->BitmapType = BitmapType::Spliced;
	bmp->Height = height;
	char* buf = memory::allocate(size);
	bmp->BmpBufPtr1 = buf;
	if (!buf)
		return -1;
	return 0;
}

int gdrv::display_palette(ColorRgba* plt)
{
	const uint32_t sysPaletteColors[]
	{
		0x00000000,
		0x00000080,
		0x00008000,
		0x00008080,
		0x00800000,
		0x00800080,
		0x00808000,
		0x00C0C0C0,
		0x00C0DCC0,
		0x00F0CAA6
	};

	memcpy(current_palette, sysPaletteColors, sizeof sysPaletteColors);

	for (int i = 0; i < 256; i++)
	{
		current_palette[i].rgba.peFlags = 0;
	}

	auto pltSrc = &plt[10];
	auto pltDst = &current_palette[10];
	for (int index = 236; index > 0; --index)
	{
		if (plt)
		{
			pltDst->rgba.peRed = pltSrc->rgba.peRed;
			pltDst->rgba.peGreen = pltSrc->rgba.peGreen;
			pltDst->rgba.peBlue = pltSrc->rgba.peBlue;
		}
		pltDst->rgba.peFlags = 4;
		pltSrc++;
		pltDst++;
	}

	current_palette[255].rgba.peBlue = -1;
	current_palette[255].rgba.peGreen = -1;
	current_palette[255].rgba.peRed = -1;

	return 0;
}


int gdrv::destroy_bitmap(gdrv_bitmap8* bmp)
{
	if (!bmp)
		return -1;

	if (bmp->BitmapType != BitmapType::None)
	{
		memory::free(bmp->BmpBufPtr1);
	}
	memset(bmp, 0, sizeof(gdrv_bitmap8));
	return 0;
}

void gdrv::start_blit_sequence()
{
}

void gdrv::end_blit_sequence()
{
}

void gdrv::blit(gdrv_bitmap8* bmp, int xSrc, int ySrc, int xDest, int yDest, int width, int height)
{
	StretchDIBitsScaled(
		xSrc,
		ySrc ,
		xDest,
		yDest,
		width,
		height,
		bmp
	);
}

void gdrv::blat(gdrv_bitmap8* bmp, int xDest, int yDest)
{
	StretchDIBitsScaled(
		0,
		0,
		xDest,
		yDest,
		bmp->Width,
		bmp->Height,
		bmp
	);
}

void gdrv::fill_bitmap(gdrv_bitmap8* bmp, int width, int height, int xOff, int yOff, char fillChar)
{
	int bmpHeight = bmp->Height;
	if (bmpHeight < 0)
		bmpHeight = -bmpHeight;
	char* bmpPtr = &bmp->BmpBufPtr1[bmp->Width * (bmpHeight - height - yOff) + xOff];
	for (; height > 0; --height)
	{
		if (width > 0)
			memset(bmpPtr, fillChar, width);
		bmpPtr += bmp->Stride;
	}
}

void gdrv::copy_bitmap(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff, gdrv_bitmap8* srcBmp,
                       int srcXOff, int srcYOff)
{
	int dstHeight = abs(dstBmp->Height);
	int srcHeight = abs(srcBmp->Height);
	char* srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * (srcHeight - height - srcYOff) + srcXOff];
	char* dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * (dstHeight - height - yOff) + xOff];

	for (int y = height; y > 0; --y)
	{
		for (int x = width; x > 0; --x)
			*dstPtr++ = *srcPtr++;

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
	}
}

void gdrv::copy_bitmap_w_transparency(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff,
                                      gdrv_bitmap8* srcBmp, int srcXOff, int srcYOff)
{
	int dstHeight = abs(dstBmp->Height);
	int srcHeight = abs(srcBmp->Height);
	char* srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * (srcHeight - height - srcYOff) + srcXOff];
	char* dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * (dstHeight - height - yOff) + xOff];

	for (int y = height; y > 0; --y)
	{
		for (int x = width; x > 0; --x)
		{
			if (*srcPtr)
				*dstPtr = *srcPtr;
			++srcPtr;
			++dstPtr;
		}

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
	}
}


void gdrv::grtext_draw_ttext_in_box(LPCSTR text, int xOff, int yOff, int width, int height, int a6)
{	
}

int gdrv::StretchDIBitsScaled(int xSrc, int ySrc, int xDst, int yDst,
                              int width, int height, gdrv_bitmap8* bmp)
{
	// Y is inverted, X normal left to right
	ySrc = max(0, min(bmp->Height - height, bmp->Height)) - ySrc;
	yDst = max(0, min(vScreenHeight - height, vScreenHeight)) - yDst;

	// Negative dst == positive src offset
	if (xDst < 0)
	{		
		xSrc -= xDst;
		xDst = 0;
	}
	if (yDst < 0)
	{
		ySrc -= yDst;
		yDst = 0;
	}

	// Clamp out of bounds rectangles
	xSrc = max(0, min(xSrc, bmp->Width));
	ySrc = max(0, min(ySrc, bmp->Height));
	if (xSrc + width > bmp->Width)
		width = bmp->Width - xSrc;
	if (ySrc + height > bmp->Height)
		height = bmp->Height - ySrc;	

	xDst = max(0, min(xDst, vScreenWidth));
	yDst = max(0, min(yDst, vScreenHeight));
	if (xDst + width > vScreenWidth)
		width = vScreenWidth - xDst;
	if (yDst + height > vScreenHeight)
		height = vScreenHeight - yDst;	

	auto srcPtr = reinterpret_cast<uint8_t*>(&bmp->BmpBufPtr1[bmp->Stride * ySrc + xSrc]);
	auto dstPtr = &reinterpret_cast<uint32_t*>(vScreenPixels)[vScreenWidth * yDst + xDst];
	for (int y = height; y > 0; --y)
	{
		for (int x = width; x > 0; --x)
		{

			*dstPtr++ = current_palette[*srcPtr++].Color;
		}

		srcPtr += bmp->Stride - width;
		dstPtr += vScreenWidth - width;
	}

	return 0;
}

void gdrv::BlitScreen()
{
	auto bmp = &render::vscreen;
	unsigned char* lockedPixels = nullptr;
	int pitch = 0;
	SDL_LockTexture
	(
		vScreenTex,
		nullptr,
		reinterpret_cast<void**>(&lockedPixels),
		&pitch
	);
	std::memcpy(lockedPixels, vScreenPixels, vScreenWidth * vScreenHeight * 4);
	SDL_UnlockTexture(vScreenTex);
	SDL_RenderCopyEx(winmain::Renderer, vScreenTex, nullptr, &DestinationRect, 0, nullptr, SDL_FLIP_VERTICAL);
}
