#include "pch.h"
#include "TBumper.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "timer.h"
#include "TPinballTable.h"

TBumper::TBumper(TPinballTable* table, int groupIndex) : TCollisionComponent2(table, groupIndex, true)
{
	visualStruct visual{};

	BmpIndex = 0;
	Timer = 0;
	TimerTime = *loader::query_float_attribute(groupIndex, 0, 407);
	loader::query_visual(groupIndex, 0, &visual);
	SoundIndex4 = visual.SoundIndex4;
	SoundIndex3 = visual.SoundIndex3;
	OriginalThreshold = Threshold;
}

int TBumper::Message2(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::TBumperSetBmpIndex:
		{
			auto nextBmp = static_cast<int>(floor(value));
			auto maxBmp = static_cast<int>(ListBitmap->size()) - 1;
			if (2 * nextBmp > maxBmp)
				nextBmp = maxBmp / 2;
			if (nextBmp < 0)
				nextBmp = 0;
			if (nextBmp != BmpIndex)
			{
				if (nextBmp >= BmpIndex)
					loader::play_sound(SoundIndex4, this, "TBumper1");
				if (nextBmp < BmpIndex)
					loader::play_sound(SoundIndex3, this, "TBumper2");
				BmpIndex = nextBmp;
				Fire();
				control::handler(11, this);
			}
			break;
		}
	case MessageCode::TBumperIncBmpIndex:
		{
			auto nextBmp = BmpIndex + 1;
			auto maxBmp = static_cast<int>(ListBitmap->size()) - 1;
			if (2 * nextBmp > maxBmp)
				nextBmp = maxBmp / 2;
			TBumper::Message2(MessageCode::TBumperSetBmpIndex, static_cast<float>(nextBmp));
			break;
		}
	case MessageCode::TBumperDecBmpIndex:
		{
			auto nextBmp = BmpIndex - 1;
			if (nextBmp < 0)
				nextBmp = 0;
			TBumper::Message2(MessageCode::TBumperSetBmpIndex, static_cast<float>(nextBmp));
			break;
		}
	case MessageCode::PlayerChanged:
		{
			auto playerPtr = &PlayerData[PinballTable->CurrentPlayer];
			playerPtr->BmpIndex = BmpIndex;
			playerPtr->MessageField = MessageField;

			playerPtr = &PlayerData[static_cast<int>(floor(value))];
			BmpIndex = playerPtr->BmpIndex;
			MessageField = playerPtr->MessageField;
			TBumper::Message2(MessageCode::TBumperSetBmpIndex, static_cast<float>(BmpIndex));
			break;
		}
	case MessageCode::Reset:
		{
			if (Timer)
			{
				timer::kill(Timer);
				TimerExpired(Timer, this);
			}
			BmpIndex = 0;
			MessageField = 0;
			for (auto& playerPtr : PlayerData)
			{
				playerPtr.BmpIndex = 0;
				playerPtr.MessageField = 0;
			}
			TimerExpired(0, this);
			break;
		}
	default:
		break;
	}

	return 0;
}

void TBumper::Collision(TBall* ball, vector2* nextPosition, vector2* direction, float distance, TEdgeSegment* edge)
{
	if (DefaultCollision(ball, nextPosition, direction))
	{
		Fire();
		control::handler(63, this);
	}
}

void TBumper::TimerExpired(int timerId, void* caller)
{
	auto bump = static_cast<TBumper*>(caller);
	auto bmp = bump->ListBitmap->at(bump->BmpIndex * 2);
	auto zMap = bump->ListZMap->at(bump->BmpIndex * 2);
	bump->Timer = 0;
	render::sprite_set(
		bump->RenderSprite,
		bmp,
		zMap,
		bmp->XPosition - bump->PinballTable->XOffset,
		bmp->YPosition - bump->PinballTable->YOffset);
	bump->Threshold = bump->OriginalThreshold;
}

void TBumper::Fire()
{
	int bmpIndex = 2 * BmpIndex + 1;
	auto bmp = ListBitmap->at(bmpIndex);
	auto zMap = ListZMap->at(bmpIndex);
	render::sprite_set(
		RenderSprite,
		bmp,
		zMap,
		bmp->XPosition - PinballTable->XOffset,
		bmp->YPosition - PinballTable->YOffset);
	Timer = timer::set(TimerTime, this, TimerExpired);
	Threshold = 1000000000.0;
}
