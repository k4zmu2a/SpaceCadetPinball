#include "pch.h"
#include "TFlipper.h"


#include "control.h"
#include "loader.h"
#include "pb.h"
#include "render.h"
#include "TFlipperEdge.h"
#include "timer.h"
#include "TZmapList.h"

TFlipper::TFlipper(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, false)
{
	visualStruct visual{};

	loader::query_visual(groupIndex, 0, &visual);
	SoundIndex1 = visual.SoundIndex4;
	SoundIndex2 = visual.SoundIndex3;
	UnknownC4F = visual.Unknown2F;
	Timer = 0;
	UnknownC5F = visual.Unknown1F;

	auto floatArr = loader::query_float_attribute(groupIndex, 0, 803);
	auto floatArr2 = loader::query_float_attribute(groupIndex, 0, 805);
	auto floatArr3 = loader::query_float_attribute(groupIndex, 0, 804);
	auto collMult = *floatArr;
	auto bmpCoef2 = *floatArr2;
	auto bmpCoef1 = *floatArr3;
	auto vecT2 = reinterpret_cast<vector_type*>(loader::query_float_attribute(groupIndex, 0, 802));
	auto vecT1 = reinterpret_cast<vector_type*>(loader::query_float_attribute(groupIndex, 0, 801));
	auto origin = reinterpret_cast<vector_type*>(loader::query_float_attribute(groupIndex, 0, 800));
	auto flipperEdge = new TFlipperEdge(
		this,
		&UnknownBaseFlag2,
		visual.Flag,
		table,
		origin,
		vecT1,
		vecT2,
		bmpCoef1,
		bmpCoef2,
		collMult,
		UnknownC4F,
		UnknownC5F);

	FlipperEdge = flipperEdge;
	if (flipperEdge)
	{
		BmpCoef1 = flipperEdge->BmpCoef1 / static_cast<float>(ListBitmap->Count() - 1);
		BmpCoef2 = flipperEdge->BmpCoef2 / static_cast<float>(ListBitmap->Count() - 1);
	}
	BmpIndex = 0;
	InputTime = 0.0;
}

TFlipper::~TFlipper()
{
	delete FlipperEdge;
}

int TFlipper::Message(int code, float value)
{
	if (code == 1 || code == 2 || code > 1008 && code <= 1011 || code == 1022)
	{
		float timerTime;
		int soundIndex = 0, code2 = code;
		if (code == 1)
		{
			control::handler(1, this);
			TimerTime = BmpCoef1;
			soundIndex = SoundIndex1;
		}
		else if (code == 2)
		{
			TimerTime = BmpCoef2;
			soundIndex = SoundIndex2;
		}
		else
		{
			code2 = 2;
			TimerTime = BmpCoef2;
		}

		if (soundIndex)
			loader::play_sound(soundIndex);
		if (Timer)
		{
			timer::kill(Timer);
			Timer = 0;
		}
		if (MessageField)
		{
			auto v10 = value - FlipperEdge->InputTime;
			timerTime = v10 - floor(v10 / TimerTime) * TimerTime;
			if (timerTime < 0.0)
				timerTime = 0.0;
		}
		else
		{
			timerTime = TimerTime;
		}
		MessageField = code2;
		InputTime = value;
		Timer = timer::set(timerTime, this, TimerExpired);
		FlipperEdge->SetMotion(code2, value);
	}

	if (code == 1020 || code == 1024)
	{
		if (MessageField)
		{
			if (Timer)
				timer::kill(Timer);
			BmpIndex = -1;
			MessageField = 2;
			TimerExpired(Timer, this);
			FlipperEdge->SetMotion(code, value);
		}
	}
	return 0;
}

void TFlipper::port_draw()
{
	FlipperEdge->port_draw();
}

void TFlipper::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
{
}

void TFlipper::TimerExpired(int timerId, void* caller)
{
	auto flip = static_cast<TFlipper*>(caller);
	int timer; // eax

	bool bmpIndexOutOfBounds = false;
	auto bmpIndexAdvance = static_cast<int>(floor((pb::time_now - flip->InputTime) / flip->TimerTime + 0.5f));
	int bmpCount = flip->ListBitmap->Count();
	if (bmpIndexAdvance > bmpCount)
		bmpIndexAdvance = bmpCount;
	if (bmpIndexAdvance < 0)
		bmpIndexAdvance = 0;

	if (!bmpIndexAdvance)
		bmpIndexAdvance = 1;

	if (flip->MessageField == 1)
	{
		flip->BmpIndex += bmpIndexAdvance;
		int countSub1 = flip->ListBitmap->Count() - 1;
		if (flip->BmpIndex >= countSub1)
		{
			flip->BmpIndex = countSub1;
			bmpIndexOutOfBounds = true;
		}
	}
	if (flip->MessageField == 2)
	{
		flip->BmpIndex -= bmpIndexAdvance;
		timer = 0;
		if (flip->BmpIndex <= 0)
		{
			flip->BmpIndex = 0;
			bmpIndexOutOfBounds = true;
		}
	}
	else
	{
		timer = 0;
	}

	if (bmpIndexOutOfBounds)
		flip->MessageField = 0;
	else
		timer = timer::set(flip->TimerTime, flip, TimerExpired);
	flip->Timer = timer;

	auto bmp = static_cast<gdrv_bitmap8*>(flip->ListBitmap->Get(flip->BmpIndex));
	auto zMap = static_cast<zmap_header_type*>(flip->ListZMap->Get(flip->BmpIndex));
	render::sprite_set(
		flip->RenderSprite,
		bmp,
		zMap,
		bmp->XPosition - flip->PinballTable->XOffset,
		bmp->YPosition - flip->PinballTable->YOffset);
}
