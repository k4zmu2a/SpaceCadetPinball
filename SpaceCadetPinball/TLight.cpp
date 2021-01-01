#include "pch.h"
#include "TLight.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "timer.h"
#include "TPinballTable.h"
#include "TZmapList.h"

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
	auto this2 = this;
	if (code > 1020)
	{
		if (code == 1024)
		{
			Reset();
			for (auto index = 0; index < this2->PinballTable->PlayerCount; ++index)
			{
				auto playerPtr = &this2->PlayerData[index];
				playerPtr->FlasherActive = this2->FlasherActive;
				playerPtr->BmpIndex2 = this2->BmpIndex2;
				playerPtr->BmpIndex1 = this2->BmpIndex1;
				playerPtr->MessageField = this2->MessageField;
			}
		}
		return 0;
	}
	if (code == 1020)
	{
		auto playerPtr = &this2->PlayerData[PinballTable->CurrentPlayer];
		playerPtr->FlasherActive = this2->FlasherActive;
		playerPtr->BmpIndex2 = this2->BmpIndex2;
		playerPtr->BmpIndex1 = this2->BmpIndex1;
		playerPtr->MessageField = this2->MessageField;

		Reset();

		playerPtr = &this2->PlayerData[static_cast<int>(floor(value))];
		this2->FlasherActive = playerPtr->FlasherActive;
		this2->BmpIndex2 = playerPtr->BmpIndex2;
		this2->BmpIndex1 = playerPtr->BmpIndex1;
		this2->MessageField = playerPtr->MessageField;
		if (this2->BmpIndex2)
		{
			this2->Message(11, static_cast<float>(this2->BmpIndex2));
		}
		if (this2->BmpIndex1)
			this2->Message(1, 0.0);
		if (this2->FlasherActive)
			this2->Message(4, 0.0);
		return 0;
	}
	switch (code)
	{
	case 0:
		this->BmpIndex1 = 0;
		if (this->FlasherActive == 0 && !this->FlasherFlag1 && !this->FlasherFlag2)
			render::sprite_set_bitmap(this->RenderSprite, this->Flasher.BmpArr[0]);
		return 0;
	case 1:
		this->BmpIndex1 = 1;
		if (this->FlasherActive == 0 && !this->FlasherFlag1 && !this->FlasherFlag2)
			render::sprite_set_bitmap(this->RenderSprite, this->Flasher.BmpArr[1]);
		return 0;
	case 2:
		break;
	case 3:
		return this->FlasherActive;
	case 4:
		schedule_timeout(0.0);
		if (!this2->FlasherActive || !this2->Flasher.Timer)
		{
			this2->FlasherActive = 1;
			this2->FlasherFlag2 = 0;
			this2->FlasherFlag1 = 0;
			this2->Unknown13 = 0;
			flasher_start(&this2->Flasher, this2->BmpIndex1);
		}
		return 0;
	case 5:
		this->Flasher.TimerDelay[0] = value * this->FlasherDelay[0];
		this->Flasher.TimerDelay[1] = value * this->FlasherDelay[1];
		return 0;
	case 6:
		this->Flasher.TimerDelay[0] = this->FlasherDelay[0];
		this->Flasher.TimerDelay[1] = this->FlasherDelay[1];
		return 0;
	case 7:
		if (!this->FlasherActive)
			flasher_start(&this->Flasher, this->BmpIndex1);
		this2->FlasherActive = 1;
		this2->FlasherFlag2 = 0;
		this2->Unknown13 = 0;
		this2->FlasherFlag1 = 0;
		schedule_timeout(value);
		return 0;
	case 8:
		if (!this->FlasherFlag1)
		{
			if (this->FlasherActive)
			{
				flasher_stop(&this->Flasher, 0);
				this2->FlasherActive = 0;
			}
			else
			{
				render::sprite_set_bitmap(this->RenderSprite, this->Flasher.BmpArr[0]);
			}
			this2->FlasherFlag1 = 1;
			this2->FlasherFlag2 = 0;
		}
		schedule_timeout(value);
		return 0;
	case 9:
		if (!this->FlasherFlag2)
		{
			if (this->FlasherActive)
			{
				flasher_stop(&this->Flasher, 1);
				this2->FlasherActive = 0;
			}
			else
			{
				render::sprite_set_bitmap(this->RenderSprite, this->Flasher.BmpArr[1]);
			}
			this2->FlasherFlag2 = 1;
			this2->FlasherFlag1 = 0;
		}
		schedule_timeout(value);
		return 0;
	case 11:
		this2->BmpIndex2 = static_cast<int>(floor(value));
		if (this2->BmpIndex2 > this2->ListBitmap->Count())
			this2->BmpIndex2 = this2->ListBitmap->Count();
		bmpIndex = 0;
		if (this2->BmpIndex2 < 0)
			this2->BmpIndex2 = 0;
		this2->Flasher.BmpArr[0] = nullptr;
		this2->Flasher.BmpArr[1] = static_cast<gdrv_bitmap8*>(this2->ListBitmap->Get(this2->BmpIndex2));
		if (this2->FlasherActive == 0)
		{
			if (!this2->FlasherFlag1)
			{
				if (this2->FlasherFlag2)
					bmpIndex = 1;
				else
					bmpIndex = this2->BmpIndex1;
			}
		}
		else
		{
			bmpIndex = this2->Flasher.BmpIndex;
		}
		render::sprite_set_bitmap(this2->RenderSprite, this2->Flasher.BmpArr[bmpIndex]);
		return 0;
	case 12:
		bmpIndex = this->BmpIndex2 + 1;
		if (bmpIndex > this->ListBitmap->Count())
			bmpIndex = this->ListBitmap->Count();
		Message(11, static_cast<float>(bmpIndex));
		return 0;
	case 13:
		bmpIndex = this->BmpIndex2 - 1;
		if (bmpIndex < 0)
			bmpIndex = 0;
		Message(11, static_cast<float>(bmpIndex));
		return 0;
	case 14:
		if (this->Timer1)
			timer::kill(this->Timer1);
		this2->Timer1 = 0;
		if (this2->FlasherActive != 0)
			flasher_stop(&this2->Flasher, -1);
		this2->FlasherActive = 0;
		this2->FlasherFlag1 = 0;
		this2->FlasherFlag2 = 0;
		render::sprite_set_bitmap(this2->RenderSprite, this2->Flasher.BmpArr[this2->BmpIndex1]);
		return 0;
	case 15:
		this->Unknown13 = 0;
		if (this->Timer2)
			timer::kill(this->Timer2);
		this2->Timer2 = 0;
		Message(1, 0.0);
		Message(7, value);
		return 0;
	case 16:
		if (this->Timer2)
			timer::kill(this->Timer2);
		this2->Timer2 = 0;
		Message(7, value);
		this2->Unknown13 = 1;
		return 0;
	case 17:
		Message(static_cast<int>(floor(value)) != 0, 0.0);
		break;
	case 18:
		Message(17, value);
		Message(14, 0.0);
		break;
	case 19:
		Message(1, 0.0);
		Message(14, 0.0);
		return 0;
	case 20:
		Message(0, 0.0);
		Message(14, 0.0);
		return 0;
	case 21:
		Message(17, static_cast<float>(this->BmpIndex1 == 0));
		break;
	case 22:
		Message(18, static_cast<float>(this->BmpIndex1 == 0));
		break;
	case 23:
		this->MessageField = static_cast<int>(floor(value));
		return 0;
	default:
		return 0;
	}

	return this2->BmpIndex1;
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
	Unknown13 = 0;
	render::sprite_set_bitmap(RenderSprite, nullptr);
	Flasher.Sprite = RenderSprite;
	Flasher.BmpArr[0] = nullptr;
	if (ListBitmap)
		Flasher.BmpArr[1] = static_cast<gdrv_bitmap8*>(ListBitmap->Get(0));
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
	if (time > 0.0)
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
	if (light->Unknown13 != 0)
	{
		light->Unknown13 = 0;
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
