#include "pch.h"
#include "zdrv.h"
#include "memory.h"
#include "pb.h"
#include "winmain.h"


int zdrv::create_zmap(zmap_header_type* zmap, int width, int height, int stride)
{
	zmap->Width = width;
	zmap->Height = height;
	zmap->Stride = stride >= 0 ? stride : pad(width);
	zmap->Texture = nullptr;

	zmap->ZPtr1 = memory::allocate<unsigned short>(zmap->Stride * zmap->Height);
	return zmap->ZPtr1 ? 0 : -1;
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
	if (zmap->Texture)
		SDL_DestroyTexture(zmap->Texture);
	memset(zmap, 0, sizeof(zmap_header_type));
	return 0;
}

void zdrv::fill(zmap_header_type* zmap, int width, int height, int xOff, int yOff, uint16_t fillWord)
{
	auto dstPtr = &zmap->ZPtr1[zmap->Stride * yOff + xOff];
	for (int y = height; y > 0; --y)
	{
		for (int x = width; x > 0; --x)
		{
			*dstPtr++ = fillWord;
		}
		dstPtr += zmap->Stride - width;
	}
}


void zdrv::paint(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff, zmap_header_type* dstZMap,
                 int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp, int srcBmpXOff, int srcBmpYOff,
                 zmap_header_type* srcZMap, int srcZMapXOff, int srcZMapYOff)
{
	assertm(srcBmp->BitmapType != BitmapTypes::Spliced, "Wrong bmp type");

	auto srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * srcBmpYOff + srcBmpXOff];
	auto dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * dstBmpYOff + dstBmpXOff];
	auto srcPtrZ = &srcZMap->ZPtr1[srcZMap->Stride * srcZMapYOff + srcZMapXOff];
	auto dstPtrZ = &dstZMap->ZPtr1[dstZMap->Stride * dstZMapYOff + dstZMapXOff];

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
                      int srcBmpYOff, uint16_t depth)
{
	assertm(srcBmp->BitmapType != BitmapTypes::Spliced, "Wrong bmp type");

	auto dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * dstBmpYOff + dstBmpXOff];
	auto srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * srcBmpYOff + srcBmpXOff];
	auto zPtr = &zMap->ZPtr1[zMap->Stride * dstZMapYOff + dstZMapXOff];

	for (int y = height; y > 0; y--)
	{
		for (int x = width; x > 0; --x)
		{
			if ((*srcPtr).Color && *zPtr > depth)
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

void zdrv::CreatePreview(zmap_header_type& zMap)
{
	if (zMap.Texture)
		return;

	auto tmpBuff = new ColorRgba[zMap.Width * zMap.Height];

	ColorRgba color{};
	auto dst = tmpBuff;
	auto src = zMap.ZPtr1;
	for (auto y = 0; y < zMap.Height; y++)
	{
		for (auto x = 0; x < zMap.Width; x++)
		{
			auto depth = static_cast<uint8_t>((0xffff - *src++) / 0xff);
			color.rgba.Blue = depth;
			color.rgba.Green = depth;
			color.rgba.Red = depth;
			*dst++ = color;
		}
		src += zMap.Stride - zMap.Width;
	}

	auto texture = SDL_CreateTexture
	(
		winmain::Renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STATIC,
		zMap.Width, zMap.Height
	);
	SDL_UpdateTexture(texture, nullptr, tmpBuff, zMap.Width * 4);
	zMap.Texture = texture;
	delete[] tmpBuff;
}

void zdrv::FlipZMapHorizontally(const zmap_header_type& zMap)
{
	// Flip in-place, iterate over Height/2 lines
	auto dst = zMap.ZPtr1;
	auto src = zMap.ZPtr1 + zMap.Stride * (zMap.Height - 1);
	for (auto y = zMap.Height - 1; y >= zMap.Height / 2; y--)
	{
		for (auto x = 0; x < zMap.Width; x++)
		{
			std::swap(*dst++, *src++);
		}
		dst += zMap.Stride - zMap.Width;
		src -= zMap.Stride + zMap.Width;
	}
}
