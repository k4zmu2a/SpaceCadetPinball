#include "pch.h"
#include "TLight.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "timer.h"
#include "TPinballTable.h"

TLight::TLight(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, true)
{
	Timer1 = 0;
	FlasherActive = 0;
	Timer2 = 0;
	Flasher.Timer = 0;
	Reset();
	float* floatArr1 = loader::query_float_attribute(groupIndex, 0, 900);
	Flasher.TimerDelay[0] = *floatArr1;
	FlasherDelay[0] = *floatArr1;
	float* floatArr2 = loader::query_float_attribute(groupIndex, 0, 901);
	Flasher.TimerDelay[1] = *floatArr2;
	FlasherDelay[1] = *floatArr2;
}

int TLight::Message(int code, float value)
{
	int bmpIndex;

	switch (code)
	{
	case 1024:
		Reset();
		for (auto index = 0; index < PinballTable->PlayerCount; ++index)
		{
			auto playerPtr = &PlayerData[index];
			playerPtr->FlasherActive = FlasherActive;
			playerPtr->BmpIndex2 = BmpIndex2;
			playerPtr->BmpIndex1 = BmpIndex1;
			playerPtr->MessageField = MessageField;
		}
		break;
	case 1020:
		{
			auto playerPtr = &PlayerData[PinballTable->CurrentPlayer];
			playerPtr->FlasherActive = FlasherActive;
			playerPtr->BmpIndex2 = BmpIndex2;
			playerPtr->BmpIndex1 = BmpIndex1;
			playerPtr->MessageField = MessageField;

			Reset();

			playerPtr = &PlayerData[static_cast<int>(floor(value))];
			FlasherActive = playerPtr->FlasherActive;
			BmpIndex2 = playerPtr->BmpIndex2;
			BmpIndex1 = playerPtr->BmpIndex1;
			MessageField = playerPtr->MessageField;
			if (BmpIndex2)
			{
				Message(11, static_cast<float>(BmpIndex2));
			}
			if (BmpIndex1)
				Message(1, 0.0);
			if (FlasherActive)
				Message(4, 0.0);
			break;
		}
	case 0:
		BmpIndex1 = 0;
		if (FlasherActive == 0 && !FlasherFlag1 && !FlasherFlag2)
			render::sprite_set_bitmap(RenderSprite, Flasher.BmpArr[0]);
		break;
	case 1:
		BmpIndex1 = 1;
		if (FlasherActive == 0 && !FlasherFlag1 && !FlasherFlag2)
			render::sprite_set_bitmap(RenderSprite, Flasher.BmpArr[1]);
		break;
	case 2:
		return BmpIndex1;
	case 3:
		return FlasherActive;
	case 4:
		schedule_timeout(0.0);
		if (!FlasherActive || !Flasher.Timer)
		{
			FlasherActive = 1;
			FlasherFlag2 = 0;
			FlasherFlag1 = 0;
			TurnOffAfterFlashingFg = 0;
			flasher_start(&Flasher, BmpIndex1);
		}
		break;
	case 5:
		Flasher.TimerDelay[0] = value * FlasherDelay[0];
		Flasher.TimerDelay[1] = value * FlasherDelay[1];
		break;
	case 6:
		Flasher.TimerDelay[0] = FlasherDelay[0];
		Flasher.TimerDelay[1] = FlasherDelay[1];
		break;
	case 7:
		if (!FlasherActive)
			flasher_start(&Flasher, BmpIndex1);
		FlasherActive = 1;
		FlasherFlag2 = 0;
		TurnOffAfterFlashingFg = 0;
		FlasherFlag1 = 0;
		schedule_timeout(value);
		break;
	case 8:
		if (!FlasherFlag1)
		{
			if (FlasherActive)
			{
				flasher_stop(&Flasher, 0);
				FlasherActive = 0;
			}
			else
			{
				render::sprite_set_bitmap(RenderSprite, Flasher.BmpArr[0]);
			}
			FlasherFlag1 = 1;
			FlasherFlag2 = 0;
		}
		schedule_timeout(value);
		break;
	case 9:
		if (!FlasherFlag2)
		{
			if (FlasherActive)
			{
				flasher_stop(&Flasher, 1);
				FlasherActive = 0;
			}
			else
			{
				render::sprite_set_bitmap(RenderSprite, Flasher.BmpArr[1]);
			}
			FlasherFlag2 = 1;
			FlasherFlag1 = 0;
		}
		schedule_timeout(value);
		break;
	case 11:
		bmpIndex = 0;
		BmpIndex2 = static_cast<int>(floor(value));
		if (BmpIndex2 > static_cast<int>(ListBitmap->size()))
			BmpIndex2 = ListBitmap->size();		
		if (BmpIndex2 < 0)
			BmpIndex2 = 0;
		Flasher.BmpArr[0] = nullptr;
		Flasher.BmpArr[1] = ListBitmap->at(BmpIndex2);
		if (FlasherActive == 0)
		{
			if (!FlasherFlag1)
			{
				if (FlasherFlag2)
					bmpIndex = 1;
				else
					bmpIndex = BmpIndex1;
			}
		}
		else
		{
			bmpIndex = Flasher.BmpIndex;
		}
		render::sprite_set_bitmap(RenderSprite, Flasher.BmpArr[bmpIndex]);
		break;
	case 12:
		bmpIndex = BmpIndex2 + 1;
		if (bmpIndex > static_cast<int>(ListBitmap->size()))
			bmpIndex = ListBitmap->size();
		Message(11, static_cast<float>(bmpIndex));
		break;
	case 13:
		bmpIndex = BmpIndex2 - 1;
		if (bmpIndex < 0)
			bmpIndex = 0;
		Message(11, static_cast<float>(bmpIndex));
		break;
	case 14:
		if (Timer1)
			timer::kill(Timer1);
		Timer1 = 0;
		if (FlasherActive != 0)
			flasher_stop(&Flasher, -1);
		FlasherActive = 0;
		FlasherFlag1 = 0;
		FlasherFlag2 = 0;
		render::sprite_set_bitmap(RenderSprite, Flasher.BmpArr[BmpIndex1]);
		break;
	case 15:
		TurnOffAfterFlashingFg = 0;
		if (Timer2)
			timer::kill(Timer2);
		Timer2 = 0;
		Message(1, 0.0);
		Message(7, value);
		break;
	case 16:
		if (Timer2)
			timer::kill(Timer2);
		Timer2 = 0;
		Message(7, value);
		TurnOffAfterFlashingFg = 1;
		break;
	case 17:
		Message(static_cast<int>(floor(value)) != 0, 0.0);
		return BmpIndex1;
	case 18:
		Message(17, value);
		Message(14, 0.0);
		return BmpIndex1;
	case 19:
		Message(1, 0.0);
		Message(14, 0.0);
		break;
	case 20:
		Message(0, 0.0);
		Message(14, 0.0);
		break;
	case 21:
		Message(17, static_cast<float>(BmpIndex1 == 0));
		return BmpIndex1;
	case 22:
		Message(18, static_cast<float>(BmpIndex1 == 0));
		return BmpIndex1;
	case 23:
		MessageField = static_cast<int>(floor(value));
		break;
	default:
		break;
	}

	return 0;
}

void TLight::Reset()
{
	if (Timer1)
		timer::kill(Timer1);
	if (Timer2)
		timer::kill(Timer2);
	if (FlasherActive)
		flasher_stop(&Flasher, -1);
	Unknown20F = 1.0;
	Timer1 = 0;
	Timer2 = 0;
	BmpIndex1 = 0;
	BmpIndex2 = 0;
	FlasherFlag1 = 0;
	FlasherFlag2 = 0;
	FlasherActive = 0;
	TurnOffAfterFlashingFg = 0;
	render::sprite_set_bitmap(RenderSprite, nullptr);
	Flasher.Sprite = RenderSprite;
	Flasher.BmpArr[0] = nullptr;
	if (ListBitmap)
		Flasher.BmpArr[1] = ListBitmap->at(0);
	Flasher.Unknown4 = 0;
	Flasher.Unknown3 = 0;
	MessageField = 0;
}

void TLight::schedule_timeout(float time)
{
	Flasher.TimerDelay[0] = FlasherDelay[0];
	Flasher.TimerDelay[1] = FlasherDelay[1];
	if (Timer1)
		timer::kill(Timer1);
	Timer1 = 0;
	if (time > 0.0f)
		Timer1 = timer::set(time, this, TimerExpired);
}

void TLight::TimerExpired(int timerId, void* caller)
{
	auto light = static_cast<TLight*>(caller);
	if (light->FlasherActive)
		flasher_stop(&light->Flasher, -1);
	render::sprite_set_bitmap(light->RenderSprite, light->Flasher.BmpArr[light->BmpIndex1]);
	light->FlasherFlag1 = 0;
	light->FlasherFlag2 = 0;
	light->FlasherActive = 0;
	light->Timer1 = 0;
	if (light->TurnOffAfterFlashingFg != 0)
	{
		light->TurnOffAfterFlashingFg = 0;
		light->Message(20, 0.0);
	}
	if (light->Control)
		control::handler(60, light);
}


void TLight::flasher_stop(flasher_type* flash, int bmpIndex)
{
	if (flash->Timer)
		timer::kill(flash->Timer);
	flash->Timer = 0;
	if (bmpIndex >= 0)
	{
		flash->BmpIndex = bmpIndex;
		render::sprite_set_bitmap(flash->Sprite, flash->BmpArr[bmpIndex]);
	}
}

void TLight::flasher_start(flasher_type* flash, int bmpIndex)
{
	flash->BmpIndex = bmpIndex;
	flasher_callback(0, flash);
}

void TLight::flasher_callback(int timerId, void* caller)
{
	auto flash = static_cast<flasher_type*>(caller);
	auto index = 1 - flash->BmpIndex;
	flash->BmpIndex = index;
	render::sprite_set_bitmap(flash->Sprite, flash->BmpArr[index]);
	flash->Timer = timer::set(flash->TimerDelay[flash->BmpIndex], flash, flasher_callback);
}
