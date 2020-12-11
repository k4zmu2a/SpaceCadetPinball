#include "pch.h"
#include "score.h"
#include "loader.h"
#include "memory.h"
#include "partman.h"
#include "winmain.h"

score_msg_font_type* score::msg_fontp;

int score::init()
{
	return 1;
}

scoreStruct* score::create(LPCSTR fieldName, gdrv_bitmap8* renderBgBmp)
{
	scoreStruct* score = (scoreStruct*)memory::allocate(sizeof(scoreStruct));
	if (!score)
		return nullptr;
	score->Unknown1 = -9999;
	score->BackgroundBmp = renderBgBmp;
	__int16* shortArr = (__int16*)partman::field_labeled(loader::loader_table, fieldName, datFieldTypes::ShortArray);
	if (!shortArr)
	{
		memory::free(score);
		return nullptr;
	}
	int groupIndex = *shortArr++;
	score->Short1 = *shortArr++;
	score->Short2 = *shortArr++;
	score->Short3 = *shortArr++;
	score->Short4 = *shortArr;
	char** bmpPtr = &score->Bitmap8Bit1;
	int index = 10;
	do
	{
		*bmpPtr = partman::field(loader::loader_table, groupIndex, datFieldTypes::Bitmap8bit);
		++bmpPtr;
		++groupIndex;
		--index;
	}
	while (index);
	return score;
}

scoreStruct* score::dup(scoreStruct* score, int scoreIndex)
{
	auto result = reinterpret_cast<scoreStruct*>(memory::allocate(sizeof(scoreStruct)));
	if (result)
		memcpy(result, score, sizeof(scoreStruct));
	return result;
}

void score::load_msg_font(LPCSTR lpName)
{
	auto resHandle = FindResourceA(winmain::hinst, lpName, RT_RCDATA);
	if (!resHandle)
		return;

	auto resGlobal = LoadResource(winmain::hinst, resHandle);
	if (!resGlobal)
		return;

	auto rcData = static_cast<__int16*>(LockResource(resGlobal));

	auto fontp = reinterpret_cast<score_msg_font_type*>(memory::allocate(sizeof(score_msg_font_type)));
	msg_fontp = fontp;
	if (!fontp)
	{
		FreeResource(resGlobal);
		return;
	}
	memset(fontp->Chars, 0, sizeof(fontp->Chars));

	auto maxWidth = 0;
	auto ptrToWidths = (char*)rcData + 6;
	for (auto index = 128; index; index--)
	{
		if (*ptrToWidths > maxWidth)
			maxWidth = *ptrToWidths;
		++ptrToWidths;
	}

	auto height = rcData[2];
	auto tmpCharBur = memory::allocate(maxWidth * height + 4);
	if (!tmpCharBur)
	{
		memory::free(msg_fontp);
		msg_fontp = nullptr;
		FreeResource(resGlobal);
		return;
	}

	msg_fontp->GapWidth = rcData[0];
	msg_fontp->Height = height;

	auto ptrToData = (char*)(rcData + 67);
	int charInd;
	for (charInd = 0; charInd < 128; charInd++)
	{
		auto width = *((char*)rcData + 6 + charInd);
		if (!width)
			continue;

		auto bmp = reinterpret_cast<gdrv_bitmap8*>(memory::allocate(sizeof(gdrv_bitmap8)));
		msg_fontp->Chars[charInd] = bmp;
		if (!bmp)
		{
			break;
		}

		if (gdrv::create_raw_bitmap(bmp, width, height, 0))
		{
			memory::free(bmp);
			msg_fontp->Chars[charInd] = nullptr;
			break;
		}

		auto sizeInBytes = height * width + 1;
		memcpy(tmpCharBur + 3, ptrToData, sizeInBytes);
		ptrToData += sizeInBytes;

		auto srcptr = tmpCharBur + 4;
		auto dstPtr = &bmp->BmpBufPtr1[bmp->Stride * (bmp->Height - 1)];
		for (auto y = 0; y < height; ++y)
		{
			memcpy(dstPtr, srcptr, width);
			srcptr += width;
			dstPtr -= bmp->Stride;
		}
	}

	if (charInd != 128)
		unload_msg_font();
	FreeResource(resGlobal);
}

void score::unload_msg_font()
{
	if (msg_fontp)
	{
		for (int i = 0; i < 128; i++)
		{
			if (msg_fontp->Chars[i])
			{
				gdrv::destroy_bitmap(msg_fontp->Chars[i]);
				memory::free(msg_fontp->Chars[i]);
			}
		}
		msg_fontp = nullptr;
	}
}
