#include "pch.h"
#include "gdrv.h"

#include "GroupData.h"
#include "partman.h"
#include "pb.h"
#include "score.h"
#include "winmain.h"

ColorRgba gdrv::current_palette[256]{};

gdrv_bitmap8::gdrv_bitmap8(int width, int height, bool indexed)
{
	assertm(width >= 0 && height >= 0, "Negative bitmap8 dimensions");

	Width = width;
	Height = height;
	Stride = width;
	IndexedStride = width;
	BitmapType = BitmapTypes::DibBitmap;
	Texture = nullptr;
	IndexedBmpPtr = nullptr;
	XPosition = 0;
	YPosition = 0;
	Resolution = 0;

	if (indexed)
		IndexedBmpPtr = new char[Height * IndexedStride];
	BmpBufPtr1 = new ColorRgba[Height * Stride];
}

gdrv_bitmap8::gdrv_bitmap8(const dat8BitBmpHeader& header)
{
	assertm(header.Width >= 0 && header.Height >= 0, "Negative bitmap8 dimensions");

	if (header.IsFlagSet(bmp8Flags::Spliced))
		BitmapType = BitmapTypes::Spliced;
	else if (header.IsFlagSet(bmp8Flags::DibBitmap))
		BitmapType = BitmapTypes::DibBitmap;
	else
		BitmapType = BitmapTypes::RawBitmap;

	Width = header.Width;
	Stride = header.Width;
	IndexedStride = header.Width;
	Height = header.Height;
	XPosition = header.XPosition;
	YPosition = header.YPosition;
	Resolution = header.Resolution;
	Texture = nullptr;

	int sizeInBytes;
	if (BitmapType == BitmapTypes::Spliced)
	{
		sizeInBytes = header.Size;
	}
	else
	{
		if (BitmapType == BitmapTypes::RawBitmap)
			assertm(Width % 4 == 0 || header.IsFlagSet(bmp8Flags::RawBmpUnaligned), "Wrong raw bitmap align flag");
		if (Width % 4)
			IndexedStride = Width - Width % 4 + 4;
		sizeInBytes = Height * IndexedStride;
		assertm(sizeInBytes == header.Size, "Wrong bitmap8 size");
	}

	IndexedBmpPtr = new char[sizeInBytes];
	BmpBufPtr1 = new ColorRgba[Stride * Height];
}

gdrv_bitmap8::~gdrv_bitmap8()
{
	if (BitmapType != BitmapTypes::None)
	{
		delete[] BmpBufPtr1;
		delete[] IndexedBmpPtr;
		if (Texture)
			SDL_DestroyTexture(Texture);
	}
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
