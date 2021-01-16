#include "pch.h"
#include "TBumper.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "timer.h"
#include "TPinballTable.h"
#include "TZmapList.h"

TBumper::TBumper(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	visualStruct visual{};

	BmpIndex = 0;
	Timer = 0;
	TimerTime = *loader::query_float_attribute(groupIndex, 0, 407);
	loader::query_visual(groupIndex, 0, &visual);
	SoundIndex4 = visual.SoundIndex4;
	SoundIndex3 = visual.SoundIndex3;
	MaxCollisionSpeed2 = MaxCollisionSpeed;
}

int TBumper::Message(int code, float value)
{
	switch (code)
	{
	case 11:
		{
			auto nextBmp = static_cast<int>(floor(value));
			if (2 * nextBmp > ListBitmap->Count() - 1)
				nextBmp = (ListBitmap->Count() - 1) / 2;
			if (nextBmp < 0)
				nextBmp = 0;
			if (nextBmp != BmpIndex)
			{
				if (nextBmp >= BmpIndex)
					loader::play_sound(SoundIndex4);
				if (nextBmp < BmpIndex)
					loader::play_sound(SoundIndex3);
				BmpIndex = nextBmp;
				Fire();
				control::handler(11, this);
			}
			break;
		}
	case 12:
		{
			auto nextBmp = BmpIndex + 1;
			auto maxBmp = ListBitmap->Count() - 1;
			if (2 * nextBmp > maxBmp)
				nextBmp = maxBmp / 2;
			TBumper::Message(11, static_cast<float>(nextBmp));
			break;
		}
	case 13:
		{
			auto nextBmp = BmpIndex - 1;
			if (nextBmp < 0)
				nextBmp = 0;
			TBumper::Message(11, static_cast<float>(nextBmp));
			break;
		}
	case 1020:
		{
			auto playerPtr = &PlayerData[PinballTable->CurrentPlayer];
			playerPtr->BmpIndex = BmpIndex;
			playerPtr->MessageField = MessageField;

			playerPtr = &PlayerData[static_cast<int>(floor(value))];
			BmpIndex = playerPtr->BmpIndex;
			MessageField = playerPtr->MessageField;
			TBumper::Message(11, static_cast<float>(BmpIndex));
			break;
		}
	case 1024:
		{
			if (Timer)
			{
				timer::kill(Timer);
				TimerExpired(Timer, this);
			}
			BmpIndex = 0;
			MessageField = 0;
			auto playerPtr = PlayerData;
			for (auto index = 0; index < PinballTable->PlayerCount; ++index)
			{
				playerPtr->BmpIndex = 0;
				playerPtr->MessageField = 0;
				++playerPtr;
			}
			TimerExpired(0, this);
			break;
		}
	default:
		break;
	}

	return 0;
}

void TBumper::Collision(TBall* ball, vector_type* nextPosition, vector_type* direction, float coef, TEdgeSegment* edge)
{
	if (DefaultCollision(ball, nextPosition, direction))
	{
		Fire();
		control::handler(63, this);
	}
}

void TBumper::put_scoring(int index, int score)
{
	if (index < 4)
		Scores[index] = score;
}


int TBumper::get_scoring(int index)
{
	return index < 4 ? Scores[index] : 0;
}

void TBumper::TimerExpired(int timerId, void* caller)
{
	auto bump = static_cast<TBumper*>(caller);
	auto bmp = static_cast<gdrv_bitmap8*>(bump->ListBitmap->Get(bump->BmpIndex * 2));
	auto zMap = static_cast<zmap_header_type*>(bump->ListZMap->Get(bump->BmpIndex * 2));
	bump->Timer = 0;
	render::sprite_set(
		bump->RenderSprite,
		bmp,
		zMap,
		bmp->XPosition - bump->PinballTable->XOffset,
		bmp->YPosition - bump->PinballTable->YOffset);
	bump->MaxCollisionSpeed = bump->MaxCollisionSpeed2;
}

void TBumper::Fire()
{
	int bmpIndex = 2 * BmpIndex + 1;
	auto bmp = static_cast<gdrv_bitmap8*>(ListBitmap->Get(bmpIndex));
	auto zMap = static_cast<zmap_header_type*>(ListZMap->Get(bmpIndex));
	render::sprite_set(
		RenderSprite,
		bmp,
		zMap,
		bmp->XPosition - PinballTable->XOffset,
		bmp->YPosition - PinballTable->YOffset);
	Timer = timer::set(TimerTime, this, TimerExpired);
	MaxCollisionSpeed = 1000000000.0;
}
