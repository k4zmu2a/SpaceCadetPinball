#include "pch.h"
#include "gdrv.h"

#include "GroupData.h"
#include "memory.h"
#include "options.h"
#include "partman.h"
#include "pb.h"
#include "score.h"
#include "winmain.h"

ColorRgba gdrv::current_palette[256]{};

int gdrv::create_bitmap(gdrv_bitmap8* bmp, int width, int height, int stride, bool indexed)
{
	assertm(width >= 0 && height >= 0, "Negative bitmap8 dimensions");

	bmp->Width = width;
	bmp->Height = height;
	bmp->Stride = width;
	bmp->BitmapType = BitmapTypes::DibBitmap;
	bmp->Texture = nullptr;

	if (stride >= 0)
		bmp->IndexedStride = stride;
	else
	{
		bmp->IndexedStride = width;
		if (width % 4)
			bmp->IndexedStride = width - width % 4 + 4;
	}

	if (indexed)
		bmp->IndexedBmpPtr = memory::allocate(bmp->Height * bmp->IndexedStride);
	bmp->BmpBufPtr1 = memory::allocate<ColorRgba>(bmp->Height * bmp->Stride);
	if (bmp->BmpBufPtr1)
	{
		return 0;
	}
	return -1;
}

int gdrv::create_bitmap(gdrv_bitmap8& bmp, const dat8BitBmpHeader& header)
{
	assertm(header.Width >= 0 && header.Height >= 0, "Negative bitmap8 dimensions");

	if (header.IsFlagSet(bmp8Flags::Spliced))
		bmp.BitmapType = BitmapTypes::Spliced;
	else if (header.IsFlagSet(bmp8Flags::DibBitmap))
		bmp.BitmapType = BitmapTypes::DibBitmap;
	else
		bmp.BitmapType = BitmapTypes::RawBitmap;

	bmp.Width = header.Width;
	bmp.Stride = header.Width;
	bmp.IndexedStride = header.Width;
	bmp.Height = header.Height;
	bmp.XPosition = header.XPosition;
	bmp.YPosition = header.YPosition;
	bmp.Resolution = header.Resolution;
	bmp.Texture = nullptr;

	int sizeInBytes;
	if (bmp.BitmapType == BitmapTypes::Spliced)
	{
		sizeInBytes = header.Size;
	}
	else
	{
		if (bmp.BitmapType == BitmapTypes::RawBitmap)
			assertm(bmp.Width % 4 == 0 || header.IsFlagSet(bmp8Flags::RawBmpUnaligned), "Wrong raw bitmap align flag");
		if (bmp.Width % 4)
			bmp.IndexedStride = bmp.Width - bmp.Width % 4 + 4;
		sizeInBytes = bmp.Height * bmp.IndexedStride;
		assertm(sizeInBytes == header.Size, "Wrong bitmap8 size");
	}

	bmp.IndexedBmpPtr = memory::allocate(sizeInBytes);
	bmp.BmpBufPtr1 = memory::allocate<ColorRgba>(bmp.Stride * bmp.Height);
	if (bmp.BmpBufPtr1)
	{
		return 0;
	}
	return -1;
}

int gdrv::display_palette(ColorRgba* plt)
{
	const uint32_t sysPaletteColors[]
	{
		0xff000000, // Color 0: transparent
		0xff000080,
		0xff008000,
		0xff008080,
		0xff800000,
		0xff800080,
		0xff808000,
		0xffC0C0C0,
		0xffC0DCC0,
		0xffF0CAA6
	};

	memcpy(current_palette, sysPaletteColors, sizeof sysPaletteColors);

	for (int i = 0; i < 256; i++)
	{
		current_palette[i].rgba.Alpha = 0;
	}

	auto pltSrc = &plt[10];
	auto pltDst = &current_palette[10];
	for (int index = 236; index > 0; --index)
	{
		if (plt)
		{
			pltDst->rgba.Blue = pltSrc->rgba.Blue;
			pltDst->rgba.Green = pltSrc->rgba.Green;
			pltDst->rgba.Red = pltSrc->rgba.Red;
		}
		pltDst->rgba.Alpha = 0xFF;
		pltSrc++;
		pltDst++;
	}

	current_palette[255].Color = 0xffFFFFFF;

	score::ApplyPalette();
	for (const auto group : pb::record_table->Groups)
	{
		for (int i = 0; i <= 2; i++)
		{
			auto bmp = group->GetBitmap(i);
			if (bmp)
			{
				ApplyPalette(*bmp);
			}
		}
	}

	return 0;
}


int gdrv::destroy_bitmap(gdrv_bitmap8* bmp)
{
	if (!bmp)
		return -1;

	if (bmp->BitmapType != BitmapTypes::None)
	{
		memory::free(bmp->BmpBufPtr1);
		if (bmp->IndexedBmpPtr)
			memory::free(bmp->IndexedBmpPtr);
		if (bmp->Texture)
			SDL_DestroyTexture(bmp->Texture);
	}
	memset(bmp, 0, sizeof(gdrv_bitmap8));
	return 0;
}

void gdrv::fill_bitmap(gdrv_bitmap8* bmp, int width, int height, int xOff, int yOff, uint8_t fillChar)
{
	auto color = current_palette[fillChar];
	auto bmpPtr = &bmp->BmpBufPtr1[bmp->Width * yOff + xOff];
	for (; height > 0; --height)
	{
		for (int x = width; x > 0; --x)
			*bmpPtr++ = color;
		bmpPtr += bmp->Stride - width;
	}
}

void gdrv::copy_bitmap(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff, gdrv_bitmap8* srcBmp,
                       int srcXOff, int srcYOff)
{
	auto srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * srcYOff + srcXOff];
	auto dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * yOff + xOff];

	for (int y = height; y > 0; --y)
	{
		std::memcpy(dstPtr, srcPtr, width * sizeof(ColorRgba));
		srcPtr += srcBmp->Stride;
		dstPtr += dstBmp->Stride;
	}
}

void gdrv::copy_bitmap_w_transparency(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff,
                                      gdrv_bitmap8* srcBmp, int srcXOff, int srcYOff)
{
	auto srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * srcYOff + srcXOff];
	auto dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * yOff + xOff];

	for (int y = height; y > 0; --y)
	{
		for (int x = width; x > 0; --x)
		{
			if ((*srcPtr).Color)
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

void gdrv::ApplyPalette(gdrv_bitmap8& bmp)
{
	if (bmp.BitmapType == BitmapTypes::None)
		return;
	assertm(bmp.BitmapType != BitmapTypes::Spliced, "gdrv: wrong bitmap type");
	assertm(bmp.IndexedBmpPtr != nullptr, "gdrv: non-indexed bitmap");

	// Apply palette, flip horizontally 
	auto dst = bmp.BmpBufPtr1;
	for (auto y = bmp.Height - 1; y >= 0; y--)
	{
		auto src = reinterpret_cast<uint8_t*>(bmp.IndexedBmpPtr) + bmp.IndexedStride * y;
		for (auto x = 0; x < bmp.Width; x++)
		{
			*dst++ = current_palette[*src++];
		}
	}
}

void gdrv::CreatePreview(gdrv_bitmap8& bmp)
{
	if (bmp.Texture)
		return;

	auto texture = SDL_CreateTexture
	(
		winmain::Renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STATIC,
		bmp.Width, bmp.Height
	);
	SDL_UpdateTexture(texture, nullptr, bmp.BmpBufPtr1, bmp.Width * 4);
	bmp.Texture = texture;
}
