#include "pch.h"
#include "score.h"
#include "loader.h"
#include "memory.h"
#include "partman.h"


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
	scoreStruct* result = (scoreStruct*)memory::allocate(sizeof(scoreStruct));
	if (result)
		memcpy(result, score, sizeof(scoreStruct));
	return result;
}
