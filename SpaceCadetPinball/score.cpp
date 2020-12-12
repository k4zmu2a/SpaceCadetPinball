#include "pch.h"
#include "score.h"
#include "loader.h"
#include "memory.h"
#include "partman.h"
#include "TDrain.h"
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
	score->Score = -9999;
	score->BackgroundBmp = renderBgBmp;
	__int16* shortArr = (__int16*)partman::field_labeled(loader::loader_table, fieldName, datFieldTypes::ShortArray);
	if (!shortArr)
	{
		memory::free(score);
		return nullptr;
	}
	int groupIndex = *shortArr++;
	score->OffsetX = *shortArr++;
	score->OffsetY = *shortArr++;
	score->Width = *shortArr++;
	score->Height = *shortArr;

	for (int index = 0; index < 10; index++)
	{
		score->CharBmp[index] = reinterpret_cast<gdrv_bitmap8*>(partman::field(
			loader::loader_table, groupIndex, datFieldTypes::Bitmap8bit));
		++groupIndex;
	}
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

void score::erase(scoreStruct* score, int blitFlag)
{
	if (score)
	{
		if (score->BackgroundBmp)
			gdrv::copy_bitmap(
				&render::vscreen,
				score->Width,
				score->Height,
				score->OffsetX,
				score->OffsetY,
				score->BackgroundBmp,
				score->OffsetX,
				score->OffsetY);
		else
			gdrv::fill_bitmap(&render::vscreen, score->Width, score->Height, score->OffsetX, score->OffsetY, 0);
		if (blitFlag)
			gdrv::blit(
				&render::vscreen,
				score->OffsetX,
				score->OffsetY,
				score->OffsetX + render::vscreen.XPosition,
				score->OffsetY + render::vscreen.YPosition,
				score->Width,
				score->Height);
	}
}

void score::set(scoreStruct* score, int value)
{
	if (score)
	{
		score->Score = value;
		score->DirtyFlag = true;
	}
}


void score::update(scoreStruct* score)
{
	char scoreBuf[12];
	if (score && score->DirtyFlag && score->Score <= 1000000000)
	{
		score->DirtyFlag = false;
		int x = score->Width + score->OffsetX;
		int y = score->OffsetY;
		erase(score, 0);
		if (score->Score >= 0)
		{
			_ltoa_s(score->Score, scoreBuf, 10);
			int len = strlen(scoreBuf);
			for (int index = len - 1; index >= 0; index--)
			{
				unsigned char curChar = scoreBuf[index];
				curChar -= '0';
				gdrv_bitmap8* bmp = score->CharBmp[curChar];
				x -= bmp->Width;
				int height = bmp->Height;
				int width = bmp->Width;
				if (render::background_bitmap)
					gdrv::copy_bitmap_w_transparency(&render::vscreen, width, height, x, y, bmp, 0, 0);
				else
					gdrv::copy_bitmap(&render::vscreen, width, height, x, y, bmp, 0, 0);
			}
		}
		gdrv::blit(
			&render::vscreen,
			score->OffsetX,
			score->OffsetY,
			score->OffsetX + render::vscreen.XPosition,
			score->OffsetY + render::vscreen.YPosition,
			score->Width,
			score->Height);
	}
}

void score::string_format(int score, char* str)
{
	CHAR separator[12];

	if (score == -999)
	{
		*str = 0;
	}
	else
	{
		lstrcpyA(separator, ",");

		HKEY phkResult;
		DWORD dwDisposition;
		if (!RegCreateKeyExA(
			HKEY_CURRENT_USER,
			"Control Panel\\International",
			0,
			nullptr,
			0,
			KEY_ALL_ACCESS,
			nullptr,
			&phkResult,
			&dwDisposition))
		{
			DWORD cbData = 10;
			RegQueryValueExA(phkResult, "sThousand", nullptr, nullptr, (LPBYTE)separator, &cbData);
			RegCloseKey(phkResult);
		}
		int scoreMillions = score % 1000000000 / 1000000;
		if (score / 1000000000 <= 0)
		{
			if (static_cast<int>(scoreMillions) <= 0)
			{
				if (score % 1000000 / 1000 <= 0)
					sprintf_s(str, 36, "%ld", score);
				else
					sprintf_s(str, 36, "%ld%s%03ld", score % 1000000 / 1000, separator, score % 1000);
			}
			else
			{
				sprintf_s(str, 36, "%ld%s%03ld%s%03ld", scoreMillions, separator, score % 1000000 / 1000, separator,
				          score % 1000);
			}
		}
		else
		{
			sprintf_s(
				str,
				36,
				"%ld%s%03ld%s%03ld%s%03ld",
				score / 1000000000,
				separator,
				scoreMillions,
				separator,
				score % 1000000 / 1000,
				separator,
				score % 1000);
		}
	}
}
