#include "pch.h"
#include "zdrv.h"
#include "memory.h"
#include "pb.h"


int zdrv::create_zmap(zmap_header_type* zmap, int width, int height)
{
	int stride = pad(width);
	zmap->Stride = stride;
	auto bmpBuf = (unsigned short*)memory::allocate(2 * height * stride);
	zmap->ZPtr1 = bmpBuf;
	if (!bmpBuf)
		return -1;
	zmap->ZPtr2 = bmpBuf;
	zmap->Width = width;
	zmap->Height = height;
	return 0;
}

int zdrv::pad(int width)
{
	int result = width;
	if (width & 3)
		result = width - (width & 3) + 4;
	return result;
}

int zdrv::destroy_zmap(zmap_header_type* zmap)
{
	if (!zmap)
		return -1;
	if (zmap->ZPtr1)
		memory::free(zmap->ZPtr1);
	memset(zmap, 0, sizeof(zmap_header_type));
	return 0;
}

void zdrv::fill(zmap_header_type* zmap, int width, int height, int xOff, int yOff, unsigned __int16 fillChar)
{
	int fillCharInt = fillChar | (fillChar << 16);
	auto zmapPtr = &zmap->ZPtr1[xOff + zmap->Stride * (zmap->Height - height - yOff)];

	for (int y = height; width > 0 && y > 0; y--)
	{
		char widthMod2 = width & 1;
		unsigned int widthDiv2 = static_cast<unsigned int>(width) >> 1;
		memset32(zmapPtr, fillCharInt, widthDiv2);

		auto lastShort = &zmapPtr[2 * widthDiv2];
		for (int i = widthMod2; i; --i)
			*lastShort++ = fillChar;

		zmapPtr += zmap->Stride;
	}
}


void zdrv::paint(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff, zmap_header_type* dstZMap,
                 int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp, int srcBmpXOff, int srcBmpYOff,
                 zmap_header_type* srcZMap, int srcZMapXOff, int srcZMapYOff)
{
	if (srcBmp->BitmapType == BitmapType::Spliced)
	{
		/*Spliced bitmap is also a zMap, how convenient*/
		paint_spliced_bmp(srcBmp->XPosition, srcBmp->YPosition, dstBmp, dstZMap, srcBmp);
		return;
	}

	int dstHeightAbs = abs(dstBmp->Height);
	int srcHeightAbs = abs(srcBmp->Height);
	auto srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * (srcHeightAbs - height - srcBmpYOff) + srcBmpXOff];
	auto dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * (dstHeightAbs - height - dstBmpYOff) + dstBmpXOff];
	auto srcPtrZ = &srcZMap->ZPtr1[srcZMap->Stride * (srcZMap->Height - height - srcZMapYOff) + srcZMapXOff];
	auto dstPtrZ = &dstZMap->ZPtr1[dstZMap->Stride * (dstZMap->Height - height - dstZMapYOff) + dstZMapXOff];

	for (int y = height; y > 0; y--)
	{
		for (int x = width; x > 0; --x)
		{
			if (*dstPtrZ >= *srcPtrZ)
			{
				*dstPtr = *srcPtr;
				*dstPtrZ = *srcPtrZ;
			}
			++srcPtr;
			++dstPtr;
			++srcPtrZ;
			++dstPtrZ;
		}

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
		srcPtrZ += srcZMap->Stride - width;
		dstPtrZ += dstZMap->Stride - width;
	}
}

void zdrv::paint_flat(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff,
                      zmap_header_type* zMap, int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp, int srcBmpXOff,
                      int srcBmpYOff, unsigned __int16 depth)
{
	int dstHeightAbs = abs(dstBmp->Height);
	int srcHeightAbs = abs(srcBmp->Height);
	auto dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * (dstHeightAbs - height - dstBmpYOff) + dstBmpXOff];
	auto srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * (srcHeightAbs - height - srcBmpYOff) + srcBmpXOff];
	auto zPtr = &zMap->ZPtr1[zMap->Stride * (zMap->Height - height - dstZMapYOff) + dstZMapXOff];

	for (int y = height; y > 0; y--)
	{
		for (int x = width; x > 0; --x)
		{
			if (*srcPtr && *zPtr > depth)
			{
				*dstPtr = *srcPtr;
			}
			++srcPtr;
			++dstPtr;
			++zPtr;
		}

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
		zPtr += zMap->Stride - width;
	}
}

void zdrv::paint_spliced_bmp(int xPos, int yPos, gdrv_bitmap8* dstBmp, zmap_header_type* dstZmap, gdrv_bitmap8* srcBmp)
{
	assertm(srcBmp->BitmapType == BitmapType::Spliced, "Wrong bmp type");
	int xOffset = xPos - pb::MainTable->XOffset;
	int yOffset = dstBmp->Height - srcBmp->Height - (yPos - pb::MainTable->YOffset);
	if (yOffset < 0)
		return;

	auto bmpDstPtr = &dstBmp->BmpBufPtr2[xOffset + yOffset * dstBmp->Stride];
	auto zMapDstPtr = &dstZmap->ZPtr2[xOffset + yOffset * dstZmap->Stride];
	auto bmpSrcPtr = reinterpret_cast<unsigned short*>(srcBmp->BmpBufPtr2);

	while (true)
	{
		auto stride = static_cast<short>(*bmpSrcPtr++);
		if (stride < 0)
			break;

		/*Stride is in terms of dst stride, hardcoded to match vScreen width in current resolution*/
		zMapDstPtr += stride;
		bmpDstPtr += stride;
		for (auto count = *bmpSrcPtr++; count; count--)
		{
			auto depth = *bmpSrcPtr++;
			auto charPtr = reinterpret_cast<char**>(&bmpSrcPtr);
			if (*zMapDstPtr >= depth)
			{
				*bmpDstPtr = **charPtr;
				*zMapDstPtr = depth;
			}

			(*charPtr)++;
			++zMapDstPtr;
			++bmpDstPtr;
		}
	}
}
