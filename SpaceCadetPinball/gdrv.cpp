#include "pch.h"
#include "gdrv.h"
#include "memory.h"
#include "pinball.h"
#include "winmain.h"

HPALETTE gdrv::palette_handle = nullptr;
HINSTANCE gdrv::hinst;
HWND gdrv::hwnd;
LOGPALETTEx256 gdrv::current_palette{};
int gdrv::sequence_handle;
HDC gdrv::sequence_hdc;
int gdrv::use_wing = 0;
int gdrv::grtext_blue = 0;
int gdrv::grtext_green = 0;
int gdrv::grtext_red = -1;


int gdrv::init(HINSTANCE hInst, HWND hWnd)
{
	hinst = hInst;
	hwnd = hWnd;
	if (!palette_handle)
		palette_handle = CreatePalette((LOGPALETTE*)&current_palette);
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


BITMAPINFO* gdrv::DibCreate(__int16 bpp, int width, int height)
{
	auto sizeBytes = height * ((width * bpp / 8 + 3) & 0xFFFFFFFC);
	auto buf = GlobalAlloc(0x42u, sizeBytes + 1064);
	auto dib = static_cast<BITMAPINFO*>(GlobalLock(buf));

	if (!dib)
		return nullptr;
	dib->bmiHeader.biSizeImage = sizeBytes;
	dib->bmiHeader.biWidth = width;
	dib->bmiHeader.biSize = 40;
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

	int index = 0;
	for (auto i = (int*)dib->bmiColors; index < static_cast<signed int>(dib->bmiHeader.biClrUsed) / 16; ++index)
	{
		*i++ = 0;
		*i++ = 0x800000;
		*i++ = 0x8000;
		*i++ = 8421376;
		*i++ = 128;
		*i++ = 8388736;
		*i++ = 32896;
		*i++ = 12632256;
		*i++ = 8421504;
		*i++ = 16711680;
		*i++ = 65280;
		*i++ = 16776960;
		*i++ = 255;
		*i++ = 16711935;
		*i++ = 0xFFFF;
		*i++ = 0xFFFFFF;
	}
	return dib;
}


void gdrv::DibSetUsage(BITMAPINFO* dib, HPALETTE hpal, int someFlag)
{
	tagPALETTEENTRY pPalEntries[256]; // [esp+4h] [ebp-400h]

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
			auto pltPtr = (short*)((char*)dib + dib->bmiHeader.biSize);
			for (int i = 0; i < numOfColors; ++i)
			{
				*pltPtr++ = i;
			}
		}
		else
		{
			assertm(false, "Entered bad code");
			char* dibPtr = (char*)dib + dib->bmiHeader.biSize;
			if (numOfColors >= 256)
				numOfColors = 256;
			GetPaletteEntries(hpal, 0, numOfColors, pPalEntries);
			int index = 0;
			char* dibPtr2 = dibPtr + 1;
			do
			{
				char v9 = pPalEntries[index++].peRed;
				dibPtr2[1] = v9;
				*dibPtr2 = dibPtr2[(char*)pPalEntries - dibPtr];
				*(dibPtr2 - 1) = dibPtr2[&pPalEntries[0].peGreen - (unsigned char*)dibPtr];
				dibPtr2[2] = 0;
				dibPtr2 += 4;
			}
			while (index < numOfColors);
		}
	}
}


int gdrv::create_bitmap_dib(gdrv_bitmap8* bmp, int width, int height)
{
	char* bmpBufPtr; // ecx
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
		bmpBufPtr = (char*)&dib->bmiHeader.biSize + 4 * dib->bmiHeader.biClrUsed + dib->bmiHeader.biSize;
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


int gdrv::display_palette(PALETTEENTRY* plt)
{
	if (palette_handle)
		DeleteObject(palette_handle);
	palette_handle = CreatePalette((LOGPALETTE*)&current_palette);
	auto windowHandle = GetDesktopWindow();
	auto dc = winmain::_GetDC(windowHandle);
	SetSystemPaletteUse(dc, 2u);
	SetSystemPaletteUse(dc, 1u);
	auto pltHandle = SelectPalette(dc, palette_handle, 0);
	RealizePalette(dc);
	SelectPalette(dc, pltHandle, 0);
	GetSystemPaletteEntries(dc, 0, 0x100u, current_palette.palPalEntry);
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
			// Todo: verify RGB order
			pltDst->peRed = pltSrc->peBlue;
			pltDst->peGreen = pltSrc->peGreen;
			pltDst->peBlue = pltSrc->peRed;
		}
		pltDst->peFlags = 4;
		pltSrc++;
		pltDst++;
	}

	if (!(GetDeviceCaps(dc, 38) & 0x100))
	{
		current_palette.palPalEntry[255].peBlue = -1;
		current_palette.palPalEntry[255].peGreen = -1;
		current_palette.palPalEntry[255].peRed = -1;
	}

	ResizePalette(palette_handle, 0x100u);
	SetPaletteEntries(palette_handle, 0, 0x100u, current_palette.palPalEntry);
	windowHandle = GetDesktopWindow();
	ReleaseDC(windowHandle, dc);
	return 0;
}


int gdrv::destroy_bitmap(gdrv_bitmap8* bmp)
{
	if (!bmp)
		return -1;
	if (bmp->BitmapType == BitmapType::RawBitmap)
	{
		memory::free(bmp->BmpBufPtr1);
	}
	else if (bmp->BitmapType == BitmapType::DibBitmap)
	{
		GlobalUnlock(GlobalHandle(bmp->Dib));
		GlobalFree(GlobalHandle(bmp->Dib));
	}
	memset(bmp, 0, sizeof(gdrv_bitmap8));
	return 0;
}

UINT gdrv::start_blit_sequence()
{
	HDC dc = winmain::_GetDC(hwnd);
	sequence_handle = 0;
	sequence_hdc = dc;
	SelectPalette(dc, palette_handle, 0);
	return RealizePalette(sequence_hdc);
}

void gdrv::blit_sequence(gdrv_bitmap8* bmp, int xSrc, int ySrcOff, int xDest, int yDest, int DestWidth, int DestHeight)
{
	if (!use_wing)
		StretchDIBits(
			sequence_hdc,
			xDest,
			yDest,
			DestWidth,
			DestHeight,
			xSrc,
			bmp->Height - ySrcOff - DestHeight,
			DestWidth,
			DestHeight,
			bmp->BmpBufPtr1,
			bmp->Dib,
			1u,
			SRCCOPY);
}


void gdrv::end_blit_sequence()
{
	ReleaseDC(hwnd, sequence_hdc);
}

void gdrv::blit(gdrv_bitmap8* bmp, int xSrc, int ySrcOff, int xDest, int yDest, int DestWidth, int DestHeight)
{
	HDC dc = winmain::_GetDC(hwnd);
	if (dc)
	{
		SelectPalette(dc, palette_handle, 0);
		RealizePalette(dc);
		if (!use_wing)
			StretchDIBits(
				dc,
				xDest,
				yDest,
				DestWidth,
				DestHeight,
				xSrc,
				bmp->Height - ySrcOff - DestHeight,
				DestWidth,
				DestHeight,
				bmp->BmpBufPtr1,
				bmp->Dib,
				1u,
				SRCCOPY);
		ReleaseDC(hwnd, dc);
	}
}

void gdrv::blat(gdrv_bitmap8* bmp, int xDest, int yDest)
{
	HDC dc = winmain::_GetDC(hwnd);
	SelectPalette(dc, palette_handle, 0);
	RealizePalette(dc);
	if (!use_wing)
		StretchDIBits(
			dc,
			xDest,
			yDest,
			bmp->Width,
			bmp->Height,
			0,
			0,
			bmp->Width,
			bmp->Height,
			bmp->BmpBufPtr1,
			bmp->Dib,
			1u,
			SRCCOPY);
	ReleaseDC(hwnd, dc);
}

void gdrv::fill_bitmap(gdrv_bitmap8* bmp, int width, int height, int xOff, int yOff, char fillChar)
{
	int bmpHeight = bmp->Height;
	if (bmpHeight < 0)
		bmpHeight = -bmpHeight;
	char* bmpPtr = &bmp->BmpBufPtr1[bmp->Width * (bmpHeight - height - yOff) + xOff];
	if (height > 0)
	{
		do
		{
			if (width > 0)
				memset(bmpPtr, fillChar, width);
			bmpPtr += bmp->Stride;
			--height;
		}
		while (height);
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
	tagRECT rc{};

	HDC dc = GetDC(hwnd);
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
	int prevMode = SetBkMode(dc, 1);
	COLORREF color = SetTextColor(dc, (grtext_red) | (grtext_green << 8) | (grtext_blue << 16));
	DrawTextA(dc, text, lstrlenA(text), &rc, 0x810u);
	SetBkMode(dc, prevMode);
	SetTextColor(dc, color);
	ReleaseDC(hwnd, dc);
}
