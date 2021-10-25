#include "pch.h"
#include "gdrv.h"

#include "fullscrn.h"
#include "memory.h"
#include "options.h"
#include "pinball.h"
#include "winmain.h"

HPALETTE gdrv::palette_handle = nullptr;
HINSTANCE gdrv::hinst;
HWND gdrv::hwnd;
int gdrv::sequence_handle;
HDC gdrv::sequence_hdc;
int gdrv::use_wing = 0;
int gdrv::grtext_blue = 0;
int gdrv::grtext_green = 0;
int gdrv::grtext_red = -1;


int gdrv::init(HINSTANCE hInst, HWND hWnd)
{
	LOGPALETTEx256 current_palette{};

	hinst = hInst;
	hwnd = hWnd;
	if (!palette_handle)
		palette_handle = CreatePalette(&current_palette);
	return 0;
}

int gdrv::uninit()
{
	if (palette_handle)
		DeleteObject(palette_handle);
	return 0;
}

void gdrv::get_focus()
{
}


BITMAPINFO* gdrv::DibCreate(int16_t bpp, int width, int height)
{
	auto sizeBytes = height * (width * bpp / 8 + 3 & (~3));
	auto dib = memory::allocate<BITMAPINFO>(1, (256 - 1) * sizeof(RGBQUAD) + sizeBytes);
	if (!dib)
		return nullptr;

	dib->bmiHeader.biSizeImage = sizeBytes;
	dib->bmiHeader.biWidth = width;
	dib->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib->bmiHeader.biHeight = height;
	dib->bmiHeader.biPlanes = 1;
	dib->bmiHeader.biBitCount = bpp;
	dib->bmiHeader.biCompression = 0;
	dib->bmiHeader.biXPelsPerMeter = 0;
	dib->bmiHeader.biYPelsPerMeter = 0;
	dib->bmiHeader.biClrUsed = 0;
	dib->bmiHeader.biClrImportant = 0;
	if (bpp == 4)
	{
		dib->bmiHeader.biClrUsed = 16;
	}
	else if (bpp == 8)
	{
		dib->bmiHeader.biClrUsed = 256;
	}


	uint32_t paletteColors[]
	{
		0, 0x800000, 0x8000, 8421376, 128, 8388736, 32896, 12632256,
		8421504, 16711680, 65280, 16776960, 255, 16711935, 0xFFFF, 0xFFFFFF,
	};
	for (auto index = 0u; index < dib->bmiHeader.biClrUsed; index += 16)
	{
		memcpy(&dib->bmiColors[index], paletteColors, sizeof paletteColors);
	}
	return dib;
}


void gdrv::DibSetUsage(BITMAPINFO* dib, HPALETTE hpal, int someFlag)
{
	tagPALETTEENTRY pPalEntries[256];

	if (!hpal)
		hpal = static_cast<HPALETTE>(GetStockObject(DEFAULT_PALETTE));
	if (!dib)
		return;
	int numOfColors = dib->bmiHeader.biClrUsed;
	if (!numOfColors)
	{
		auto bpp = dib->bmiHeader.biBitCount;
		if (bpp <= 8u)
			numOfColors = 1 << bpp;
	}
	if (numOfColors > 0 && (dib->bmiHeader.biCompression != 3 || numOfColors == 3))
	{
		if (someFlag && someFlag <= 2)
		{
			auto pltPtr = reinterpret_cast<short*>(dib->bmiColors);
			for (auto i = 0; i < numOfColors; ++i)
			{
				*pltPtr++ = i;
			}
		}
		else
		{
			if (numOfColors >= 256)
				numOfColors = 256;
			GetPaletteEntries(hpal, 0, numOfColors, pPalEntries);
			for (auto index = 0; index < numOfColors; index++)
			{
				dib->bmiColors[index].rgbRed = pPalEntries[index].peRed;
				dib->bmiColors[index].rgbGreen = pPalEntries[index].peGreen;
				dib->bmiColors[index].rgbBlue = pPalEntries[index].peBlue;
				dib->bmiColors[index].rgbReserved = 0;
			}
		}
	}
}


int gdrv::create_bitmap_dib(gdrv_bitmap8* bmp, int width, int height)
{
	char* bmpBufPtr;
	auto dib = DibCreate(8, width, height);
	DibSetUsage(dib, palette_handle, 1);

	bmp->Dib = dib;
	bmp->Width = width;
	bmp->Stride = width;
	if (width % 4)
		bmp->Stride = 4 - width % 4 + width;

	bmp->Height = height;
	bmp->BitmapType = BitmapType::DibBitmap;

	if (dib->bmiHeader.biCompression == 3)
		bmpBufPtr = (char*)&dib->bmiHeader.biPlanes + dib->bmiHeader.biSize;
	else
		bmpBufPtr = reinterpret_cast<char*>(&dib->bmiColors[dib->bmiHeader.biClrUsed]);
	bmp->BmpBufPtr1 = bmpBufPtr;
	bmp->BmpBufPtr2 = bmpBufPtr;
	return 0;
}

int gdrv::create_bitmap(gdrv_bitmap8* bmp, int width, int height)
{
	return create_bitmap_dib(bmp, width, height);
}

int gdrv::create_raw_bitmap(gdrv_bitmap8* bmp, int width, int height, int flag)
{
	bmp->Dib = nullptr;
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
	bmp->BmpBufPtr2 = buf;
	return 0;
}

int gdrv::create_spliced_bitmap(gdrv_bitmap8* bmp, int width, int height, int size)
{
	bmp->Dib = nullptr;
	bmp->Width = width;
	bmp->Stride = width;
	bmp->BitmapType = BitmapType::Spliced;
	bmp->Height = height;
	char* buf = memory::allocate(size);
	bmp->BmpBufPtr1 = buf;
	if (!buf)
		return -1;
	bmp->BmpBufPtr2 = bmp->BmpBufPtr1;
	return 0;
}


int gdrv::display_palette(PALETTEENTRY* plt)
{
	LOGPALETTEx256 current_palette{};

	if (palette_handle)
		DeleteObject(palette_handle);
	palette_handle = CreatePalette(&current_palette);
	auto windowHandle = GetDesktopWindow();
	auto dc = winmain::_GetDC(windowHandle);
	SetSystemPaletteUse(dc, 2u);
	SetSystemPaletteUse(dc, 1u);
	auto originalPalette = SelectPalette(dc, palette_handle, 0);
	RealizePalette(dc);
	SelectPalette(dc, originalPalette, 0);
	GetSystemPaletteEntries(dc, 0, 256, current_palette.palPalEntry);
	for (int i = 0; i < 256; i++)
	{
		current_palette.palPalEntry[i].peFlags = 0;
	}

	auto pltSrc = &plt[10];
	auto pltDst = &current_palette.palPalEntry[10];
	for (int index = 236; index > 0; --index)
	{
		if (plt)
		{
			pltDst->peRed = pltSrc->peBlue;
			pltDst->peGreen = pltSrc->peGreen;
			pltDst->peBlue = pltSrc->peRed;
		}
		pltDst->peFlags = 4;
		pltSrc++;
		pltDst++;
	}

	if (!(GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE))
	{
		current_palette.palPalEntry[255].peBlue = -1;
		current_palette.palPalEntry[255].peGreen = -1;
		current_palette.palPalEntry[255].peRed = -1;
	}

	ResizePalette(palette_handle, 256);
	SetPaletteEntries(palette_handle, 0, 256, current_palette.palPalEntry);
	windowHandle = GetDesktopWindow();
	ReleaseDC(windowHandle, dc);
	return 0;
}


int gdrv::destroy_bitmap(gdrv_bitmap8* bmp)
{
	if (!bmp)
		return -1;
	if (bmp->BitmapType == BitmapType::RawBitmap || bmp->BitmapType == BitmapType::Spliced)
	{
		memory::free(bmp->BmpBufPtr1);
	}
	else if (bmp->BitmapType == BitmapType::DibBitmap)
	{
		memory::free(bmp->Dib);
	}
	memset(bmp, 0, sizeof(gdrv_bitmap8));
	return 0;
}

void gdrv::start_blit_sequence()
{
	HDC dc = winmain::_GetDC(hwnd);
	sequence_handle = 0;
	sequence_hdc = dc;
	SelectPalette(dc, palette_handle, 0);
	RealizePalette(sequence_hdc);
	SetStretchBltMode(dc, stretchMode);
}

void gdrv::blit_sequence(gdrv_bitmap8* bmp, int xSrc, int ySrcOff, int xDest, int yDest, int DestWidth, int DestHeight)
{
	if (!use_wing)
		StretchDIBitsScaled(
			sequence_hdc,
			xDest,
			yDest,
			DestWidth,
			DestHeight,
			xSrc,
			bmp->Height - ySrcOff - DestHeight,
			DestWidth,
			DestHeight,
			bmp,
			DIB_PAL_COLORS,
			SRCCOPY
		);
}


void gdrv::end_blit_sequence()
{
	ReleaseDC(hwnd, sequence_hdc);
}

void gdrv::blit(gdrv_bitmap8* bmp, int xSrc, int ySrc, int xDest, int yDest, int DestWidth, int DestHeight)
{
	HDC dc = winmain::_GetDC(hwnd);
	SetStretchBltMode(dc, stretchMode);
	if (dc)
	{
		SelectPalette(dc, palette_handle, 0);
		RealizePalette(dc);
		if (!use_wing)
			StretchDIBitsScaled(
				dc,
				xDest,
				yDest,
				DestWidth,
				DestHeight,
				xSrc,
				bmp->Height - ySrc - DestHeight,
				DestWidth,
				DestHeight,
				bmp,
				DIB_PAL_COLORS,
				SRCCOPY
			);
		ReleaseDC(hwnd, dc);
	}
}

void gdrv::blat(gdrv_bitmap8* bmp, int xDest, int yDest)
{
	HDC dc = winmain::_GetDC(hwnd);

	// Black background for uniform scaling
	if (fullscrn::OffsetX > 0 || fullscrn::OffsetY > 0)
	{
		const auto rop = BLACKNESS;
		RECT client{};
		GetClientRect(hwnd, &client);
		if (fullscrn::OffsetX > 0)
		{
			BitBlt(dc, 0, 0, min(fullscrn::OffsetX + 2, client.right), client.bottom, dc, 0, 0, rop);
			BitBlt(dc, max(client.right - fullscrn::OffsetX - 2, 0), 0, client.right, client.bottom, dc, 0, 0, rop);
		}
		else
		{
			BitBlt(dc, 0, 0, client.right, min(fullscrn::OffsetY + 2, client.bottom), dc, 0, 0, rop);
			BitBlt(dc, 0, max(client.bottom - fullscrn::OffsetY - 2, 0), client.right, client.bottom, dc, 0, 0, rop);
		}
	}

	SelectPalette(dc, palette_handle, 0);
	RealizePalette(dc);
	SetStretchBltMode(dc, stretchMode);
	if (!use_wing)
		StretchDIBitsScaled(
			dc,
			xDest,
			yDest,
			bmp->Width,
			bmp->Height,
			0,
			0,
			bmp->Width,
			bmp->Height,
			bmp,
			DIB_PAL_COLORS,
			SRCCOPY
		);
	ReleaseDC(hwnd, dc);
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


void gdrv::grtext_draw_ttext_in_box(LPCSTR text, int xOff, int yOff, int width, int height)
{
	// Original font was 16 points, used with lowest table resolution
	static const int fontSizes[3] =
	{
		16,
		22,
		28
	};

	xOff = static_cast<int>(xOff * fullscrn::ScaleX) + fullscrn::OffsetX;
	yOff = static_cast<int>(yOff * fullscrn::ScaleY) + fullscrn::OffsetY;
	width = static_cast<int>(width * fullscrn::ScaleX);
	height = static_cast<int>(height * fullscrn::ScaleY);
	auto fontSize = static_cast<int>(round(fontSizes[fullscrn::GetResolution()] * fullscrn::ScaleY));

	HDC dc = GetDC(hwnd);
	tagRECT rc{};
	rc.left = xOff;
	rc.right = width + xOff;
	rc.top = yOff;
	rc.bottom = height + yOff;
	if (grtext_red < 0)
	{
		grtext_blue = 255;
		grtext_green = 255;
		grtext_red = 255;
		const char* fontColor = pinball::get_rc_string(189, 0);
		if (fontColor)
			sscanf_s(fontColor, "%d %d %d", &grtext_red, &grtext_green, &grtext_blue);
	}

	char font[30];
	switch (options::Options.Language)
	{
	case Languages::TraditionalChinese:
		strcpy_s(font, "Microsoft JhengHei");
		break;
	case Languages::SimplifiedChinese:
		strcpy_s(font, "Microsoft YaHei");
		break;
	default:
		strcpy_s(font, "Arial");
	}

	// DEFAULT_CHARSET in unicode build.
	// Default font does not scale well
	auto hNewFont = CreateFont(fontSize, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE,
	                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	                           DEFAULT_PITCH | FF_SWISS, font);
	HFONT hOldFont = static_cast<HFONT>(SelectObject(dc, hNewFont));
	int prevMode = SetBkMode(dc, TRANSPARENT);
	COLORREF color = SetTextColor(dc, grtext_red | grtext_green << 8 | grtext_blue << 16);

	DrawTextA(dc, text, lstrlenA(text), &rc, DT_NOPREFIX | DT_WORDBREAK);

	SelectObject(dc, hOldFont);
	DeleteObject(hNewFont);
	SetBkMode(dc, prevMode);
	SetTextColor(dc, color);
	ReleaseDC(hwnd, dc);
}

int gdrv::StretchDIBitsScaled(HDC hdc, int xDest, int yDest, int DestWidth, int DestHeight, int xSrc, int ySrc,
                              int SrcWidth, int SrcHeight, gdrv_bitmap8* bmp, UINT iUsage,
                              DWORD rop)
{
	/*Scaled partial updates may leave 1px border artifacts around update area.
	 * Pad update area to compensate.*/
	const int pad = 1, padX2 = pad * 2;
	if (fullscrn::ScaleX > 1 && xSrc > pad && xSrc + pad < bmp->Width)
	{
		xSrc -= pad;
		xDest -= pad;
		SrcWidth += padX2;
		DestWidth += padX2;
	}

	if (fullscrn::ScaleY > 1 && ySrc > pad && ySrc + pad < bmp->Height)
	{
		ySrc -= pad;
		yDest -= pad;
		SrcHeight += padX2;
		DestHeight += padX2;
	}

	return StretchDIBits(
		hdc,
		static_cast<int>(round(xDest * fullscrn::ScaleX + fullscrn::OffsetX)),
		static_cast<int>(round(yDest * fullscrn::ScaleY + fullscrn::OffsetY)),
		static_cast<int>(round(DestWidth * fullscrn::ScaleX)),
		static_cast<int>(round(DestHeight * fullscrn::ScaleY)),
		xSrc,
		ySrc,
		SrcWidth,
		SrcHeight,
		bmp->BmpBufPtr1,
		bmp->Dib,
		iUsage,
		rop);
}
