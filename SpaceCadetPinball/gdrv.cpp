#include "pch.h"
#include "gdrv.h"

#include "GroupData.h"
#include "partman.h"
#include "pb.h"
#include "score.h"
#include "winmain.h"
#include "TTextBox.h"
#include "fullscrn.h"

ColorRgba gdrv::current_palette[256]{};

gdrv_bitmap8::gdrv_bitmap8(int width, int height) : gdrv_bitmap8(width, height, true, true)
{
}

gdrv_bitmap8::gdrv_bitmap8(int width, int height, bool indexed) : gdrv_bitmap8(width, height, indexed, true)
{
}

gdrv_bitmap8::gdrv_bitmap8(int width, int height, bool indexed, bool bmpBuff)
{
	assertm(width >= 0 && height >= 0, "Negative bitmap8 dimensions");

	Width = width;
	Height = height;
	Stride = width;
	IndexedStride = width;
	BitmapType = BitmapTypes::DibBitmap;
	Texture = nullptr;
	IndexedBmpPtr = nullptr;
	BmpBufPtr1 = nullptr;
	XPosition = 0;
	YPosition = 0;
	Resolution = 0;

	if (indexed)
		IndexedBmpPtr = new char[Height * IndexedStride];
	if (bmpBuff)
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

void gdrv_bitmap8::ScaleIndexed(float scaleX, float scaleY)
{
	if (!IndexedBmpPtr)
	{
		assertm(false, "Scaling non-indexed bitmap");
		return;
	}

	int newWidht = static_cast<int>(Width * scaleX), newHeight = static_cast<int>(Height * scaleY);
	if (Width == newWidht && Height == newHeight)
		return;

	auto newIndBuf = new char[newHeight * newWidht];
	for (int dst = 0, y = 0; y < newHeight; y++)
	{
		for (int x = 0; x < newWidht; x++, dst++)
		{
			auto px = static_cast<int>(x / scaleX);
			auto py = static_cast<int>(y / scaleY);
			newIndBuf[dst] = IndexedBmpPtr[(py * IndexedStride) + px];
		}
	}

	Stride = IndexedStride = Width = newWidht;
	Height = newHeight;

	delete IndexedBmpPtr;
	IndexedBmpPtr = newIndBuf;
	delete BmpBufPtr1;
	BmpBufPtr1 = new ColorRgba[Stride * Height];
}

void gdrv_bitmap8::CreateTexture(const char* scaleHint, int access)
{
	if (Texture != nullptr)
	{
		SDL_DestroyTexture(Texture);
	}

	UsingSdlHint hint{ SDL_HINT_RENDER_SCALE_QUALITY, scaleHint };
	Texture = SDL_CreateTexture
	(
		winmain::Renderer,
		SDL_PIXELFORMAT_BGRA32,
		access,
		Width, Height
	);
	SDL_SetTextureBlendMode(Texture, SDL_BLENDMODE_NONE);
}

void gdrv_bitmap8::BlitToTexture()
{
	assertm(Texture, "Updating null texture");
	int pitch = 0;
	ColorRgba* lockedPixels;
	auto result = SDL_LockTexture
	(
		Texture,
		nullptr,
		reinterpret_cast<void**>(&lockedPixels),
		&pitch
	);
	assertm(result == 0, "Updating non-streaming texture");
	assertm(static_cast<unsigned>(pitch) == Width * sizeof(ColorRgba), "Padding on vScreen texture");

	std::memcpy(lockedPixels, BmpBufPtr1, Width * Height * sizeof(ColorRgba));

	SDL_UnlockTexture(Texture);
}

int gdrv::display_palette(ColorRgba* plt)
{
	// Colors from Windows system palette
	const ColorRgba sysPaletteColors[10]
	{
		ColorRgba{0, 0, 0, 0}, // Color 0: transparent
		ColorRgba{0x80, 0, 0, 0xff},
		ColorRgba{0, 0x80, 0, 0xff},
		ColorRgba{0x80, 0x80, 0, 0xff},
		ColorRgba{0, 0, 0x80, 0xff},
		ColorRgba{0x80, 0, 0x80, 0xff},
		ColorRgba{0, 0x80, 0x80, 0xff},
		ColorRgba{0xC0, 0xC0, 0xC0, 0xff},
		ColorRgba{0xC0, 0xDC, 0xC0, 0xff},
		ColorRgba{0xA6, 0xCA, 0xF0, 0xff},
	};

	std::memset(current_palette, 0, sizeof current_palette);
	std::memcpy(current_palette, sysPaletteColors, sizeof sysPaletteColors);	

	for (int index = 10; plt && index < 246; index++)
	{
		auto srcClr = plt[index];
		srcClr.SetAlpha(0xff);		
		current_palette[index] = ColorRgba{ srcClr };
		current_palette[index].SetAlpha(2);
	}

	current_palette[255] = ColorRgba::White();

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
	fill_bitmap(bmp, width, height, xOff, yOff, current_palette[fillChar]);
}

void gdrv::fill_bitmap(gdrv_bitmap8* bmp, int width, int height, int xOff, int yOff, ColorRgba fillColor)
{
	auto bmpPtr = &bmp->BmpBufPtr1[bmp->Width * yOff + xOff];
	for (; height > 0; --height)
	{
		for (int x = width; x > 0; --x)
			*bmpPtr++ = fillColor;
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

void gdrv::ScrollBitmapHorizontal(gdrv_bitmap8* bmp, int xStart)
{
	auto srcPtr = bmp->BmpBufPtr1;
	auto startOffset = xStart >= 0 ? 0 : -xStart;
	auto endOffset = xStart >= 0 ? xStart : 0;
	auto length = bmp->Width - std::abs(xStart);
	for (int y = bmp->Height; y > 0; --y)
	{
		std::memmove(srcPtr + endOffset, srcPtr + startOffset, length * sizeof(ColorRgba));
		srcPtr += bmp->Stride;
	}
}


void gdrv::grtext_draw_ttext_in_box(TTextBox* textBox)
{
	// Do nothing when using a font (the text will be rendered in TTextBox::Draw)
	if(textBox->Font)
		return;

	char windowName[64];
	SDL_Rect rect;
	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoInputs;
	
	rect.x = textBox->OffsetX;
	rect.y = textBox->OffsetY;
	rect.w = textBox->Width;
	rect.h = textBox->Height;

	rect = fullscrn::GetScreenRectFromPinballRect(rect);

	ImGui::SetNextWindowPos(ImVec2(rect.x, rect.y));
	ImGui::SetNextWindowSize(ImVec2(rect.w, rect.h));

	// Use the pointer to generate a window unique name per text box
	snprintf(windowName, sizeof(windowName), "TTextBox_%p", textBox);
	if (ImGui::Begin(windowName, nullptr, window_flags))
	{
		ImGui::SetWindowFontScale(fullscrn::GetScreenToPinballRatio());
		if(textBox->Message1)
			ImGui::TextWrapped("%s", textBox->Message1->Text);
	}
	ImGui::End();
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

	bmp.CreateTexture("nearest", SDL_TEXTUREACCESS_STATIC);
	SDL_UpdateTexture(bmp.Texture, nullptr, bmp.BmpBufPtr1, bmp.Width * 4);
}
