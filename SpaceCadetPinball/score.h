#pragma once
#include "gdrv.h"

struct scoreStruct
{
	int Unknown1;
	int Unknown2;
	gdrv_bitmap8* BackgroundBmp;
	int Short1;
	int Short2;
	int Short3;
	int Short4;
	char* Bitmap8Bit1;
	char* Bitmap8Bit2;
	char* Bitmap8Bit3;
	char* Bitmap8Bit4;
	char* Bitmap8Bit5;
	char* Bitmap8Bit6;
	char* Bitmap8Bit7;
	char* Bitmap8Bit8;
	char* Bitmap8Bit9;
	char* Bitmap8Bit10;
};

struct score_msg_font_type
{
	int GapWidth;
	int Height;
	gdrv_bitmap8* Chars[128];
};

struct score_font_rc
{
	short Header0;
	short Header1;
	short Height;
	char SomeLen[128];
};


class score
{
public:
	static score_msg_font_type* msg_fontp;
	static int init();
	static scoreStruct* create(LPCSTR fieldName, gdrv_bitmap8* renderBgBmp);
	static scoreStruct* dup(scoreStruct* score, int scoreIndex);
	static void load_msg_font(LPCSTR lpName);
	static void unload_msg_font();
};
