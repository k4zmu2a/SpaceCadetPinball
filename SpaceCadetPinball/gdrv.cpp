#include "pch.h"
#include "gdrv.h"
#include "memory.h"

HPALETTE gdrv::palette_handle=0;

void gdrv::get_focus()
{
}


gdrv_dib* gdrv::DibCreate(__int16 bpp, int width, int height)
{
	auto sizeBytes = height * ((width * bpp / 8 + 3) & 0xFFFFFFFC);
	auto buf = GlobalAlloc(0x42u, sizeBytes + 1064);
	auto dib = static_cast<gdrv_dib*>(GlobalLock(buf));

	if (!dib)
		return nullptr;
	dib->BufferSize = sizeBytes;
	dib->Width = width;
	dib->PaletteOffset = 40;
	dib->Height = height;
	dib->Unknown3_1 = 1;
	dib->Bpp = bpp;
	dib->Unknown4 = 0;
	dib->Unknown6 = 0;
	dib->Unknown7 = 0;
	dib->NumberOfColors = 0;
	dib->Unknown9 = 0;
	if (bpp == 4)
	{
		dib->NumberOfColors = 16;
	}
	else if (bpp == 8)
	{
		dib->NumberOfColors = 256;
	}

	auto pltPtr = &dib->Palette0;
	for (auto index = 0; index < dib->NumberOfColors / 16; ++index, pltPtr++)
	{
		*pltPtr = gdrv_dib_palette{
			{0},
			{0x800000},
			{0x8000},
			{8421376},
			{128},
			{8388736},
			{32896},
			{12632256},
			{8421504},
			{16711680},
			{65280},
			{16776960},
			{255},
			{16711935},
			{0xFFFF},
			{0xFFFFFF},
		};
	}
	return dib;
}


void gdrv::DibSetUsage(gdrv_dib* dib, HPALETTE hpal, int someFlag)
{
	tagPALETTEENTRY pPalEntries[256]; // [esp+4h] [ebp-400h]

	if (!hpal)
		hpal = static_cast<HPALETTE>(GetStockObject(DEFAULT_PALETTE));
	if (!dib)
		return;
	int numOfColors = dib->NumberOfColors;
	if (!numOfColors)
	{
		auto bpp = dib->Bpp;
		if (bpp <= 8u)
			numOfColors = 1 << bpp;
	}
	if (numOfColors > 0 && (dib->Unknown4 != 3 || numOfColors == 3))
	{
		if (someFlag && someFlag <= 2)
		{
			auto pltPtr = (short*)((char*)dib + dib->PaletteOffset);
			for (int i = 0; i < numOfColors; ++i)
			{
				*pltPtr++ = i;
			}
		}
		else
		{
			assertm(false, "Entered bad code");
			char* dibPtr = (char*)dib + dib->PaletteOffset;
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
	gdrv_dib* dib = DibCreate(8, width, height);
	DibSetUsage(dib, palette_handle, 1);

	bmp->Dib = dib;
	bmp->Width = width;
	bmp->Stride = width;
	if (width % 4)
		bmp->Stride = 4 - width % 4 + width;
	gdrv_dib* dib2 = bmp->Dib;
	bmp->Height = height;
	bmp->SomeByte = 2;

	if (dib2->Unknown4 == 3)
		bmpBufPtr = (char*)&dib2->Unknown3_1 + dib2->PaletteOffset;
	else
		bmpBufPtr = (char*)&dib2->PaletteOffset + 4 * dib2->NumberOfColors + dib2->PaletteOffset;
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
	bmp->SomeByte = 1;
	char* buf = memory::allocate(sizeInBytes);
	bmp->BmpBufPtr1 = buf;
	if (!buf)
		return -1;
	bmp->BmpBufPtr2 = buf;
	return 0;
}
