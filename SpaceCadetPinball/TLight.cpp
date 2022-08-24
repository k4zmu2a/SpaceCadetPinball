#include "pch.h"
#include "TLight.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "timer.h"
#include "TPinballTable.h"

TLight::TLight(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, true)
{
	TimeoutTimer = 0;
	FlasherOnFlag = false;
	UndoOverrideTimer = 0;
	FlashTimer = 0;
	Reset();
	float* floatArr1 = loader::query_float_attribute(groupIndex, 0, 900);
	FlashDelay[0] = *floatArr1;
	SourceDelay[0] = *floatArr1;
	float* floatArr2 = loader::query_float_attribute(groupIndex, 0, 901);
	FlashDelay[1] = *floatArr2;
	SourceDelay[1] = *floatArr2;
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
			playerPtr->FlasherOnFlag = FlasherOnFlag;
			playerPtr->LightOnBmpIndex = LightOnBmpIndex;
			playerPtr->LightOnFlag = LightOnFlag;
			playerPtr->MessageField = MessageField;
		}
		break;
	case 1020:
		{
			auto playerPtr = &PlayerData[PinballTable->CurrentPlayer];
			playerPtr->FlasherOnFlag = FlasherOnFlag;
			playerPtr->LightOnBmpIndex = LightOnBmpIndex;
			playerPtr->LightOnFlag = LightOnFlag;
			playerPtr->MessageField = MessageField;

			Reset();

			playerPtr = &PlayerData[static_cast<int>(floor(value))];
			FlasherOnFlag = playerPtr->FlasherOnFlag;
			LightOnBmpIndex = playerPtr->LightOnBmpIndex;
			LightOnFlag = playerPtr->LightOnFlag;
			MessageField = playerPtr->MessageField;
			if (LightOnBmpIndex)
			{
				Message(11, static_cast<float>(LightOnBmpIndex));
			}
			if (LightOnFlag)
				Message(1, 0.0);
			if (FlasherOnFlag)
				Message(4, 0.0);
			break;
		}
	case 0:
		LightOnFlag = false;
		if (!FlasherOnFlag && !ToggledOffFlag && !ToggledOnFlag)
			SetSpriteBmp(BmpArr[0]);
		break;
	case 1:
		LightOnFlag = true;
		if (!FlasherOnFlag && !ToggledOffFlag && !ToggledOnFlag)
			SetSpriteBmp(BmpArr[1]);
		break;
	case 2:
		return LightOnFlag;
	case 3:
		return FlasherOnFlag;
	case 4:
		schedule_timeout(0.0);
		if (!FlasherOnFlag || !FlashTimer)
		{
			FlasherOnFlag = true;
			ToggledOnFlag = false;
			ToggledOffFlag = false;
			TurnOffAfterFlashingFg = false;
			flasher_start(LightOnFlag);
		}
		break;
	case 5:
		FlashDelay[0] = value * SourceDelay[0];
		FlashDelay[1] = value * SourceDelay[1];
		break;
	case 6:
		FlashDelay[0] = SourceDelay[0];
		FlashDelay[1] = SourceDelay[1];
		break;
	case 7:
		if (!FlasherOnFlag)
			flasher_start(LightOnFlag);
		FlasherOnFlag = true;
		ToggledOnFlag = false;
		TurnOffAfterFlashingFg = false;
		ToggledOffFlag = false;
		schedule_timeout(value);
		break;
	case 8:
		if (!ToggledOffFlag)
		{
			if (FlasherOnFlag)
			{
				flasher_stop(0);
				FlasherOnFlag = false;
			}
			else
			{
				SetSpriteBmp(BmpArr[0]);
			}
			ToggledOffFlag = true;
			ToggledOnFlag = false;
		}
		schedule_timeout(value);
		break;
	case 9:
		if (!ToggledOnFlag)
		{
			if (FlasherOnFlag)
			{
				flasher_stop(1);
				FlasherOnFlag = false;
			}
			else
			{
				SetSpriteBmp(BmpArr[1]);
			}
			ToggledOnFlag = true;
			ToggledOffFlag = false;
		}
		schedule_timeout(value);
		break;
	case 11:
		LightOnBmpIndex = Clamp(static_cast<int>(floor(value)), 0, static_cast<int>(ListBitmap->size()) - 1);
		BmpArr[0] = nullptr;
		BmpArr[1] = ListBitmap->at(LightOnBmpIndex);
		if (!FlasherOnFlag)
		{
			if (ToggledOffFlag)
				bmpIndex = 0;
			else if (ToggledOnFlag)
				bmpIndex = 1;
			else
				bmpIndex = LightOnFlag;
		}
		else
		{
			bmpIndex = FlashLightOnFlag;
		}
		SetSpriteBmp(BmpArr[bmpIndex]);
		break;
	case 12:
		bmpIndex = LightOnBmpIndex + 1;
		if (bmpIndex >= static_cast<int>(ListBitmap->size()))
			bmpIndex = static_cast<int>(ListBitmap->size()) - 1;
		Message(11, static_cast<float>(bmpIndex));
		break;
	case 13:
		bmpIndex = LightOnBmpIndex - 1;
		if (bmpIndex < 0)
			bmpIndex = 0;
		Message(11, static_cast<float>(bmpIndex));
		break;
	case 14:
		if (TimeoutTimer)
			timer::kill(TimeoutTimer);
		TimeoutTimer = 0;
		if (FlasherOnFlag)
			flasher_stop(-1);
		FlasherOnFlag = false;
		ToggledOffFlag = false;
		ToggledOnFlag = false;
		SetSpriteBmp(BmpArr[LightOnFlag]);
		break;
	case 15:
		TurnOffAfterFlashingFg = false;
		if (UndoOverrideTimer)
			timer::kill(UndoOverrideTimer);
		UndoOverrideTimer = 0;
		Message(1, 0.0);
		Message(7, value);
		break;
	case 16:
		if (UndoOverrideTimer)
			timer::kill(UndoOverrideTimer);
		UndoOverrideTimer = 0;
		Message(7, value);
		TurnOffAfterFlashingFg = true;
		break;
	case 17:
		Message(static_cast<int>(floor(value)) != 0, 0.0);
		return LightOnFlag;
	case 18:
		Message(17, value);
		Message(14, 0.0);
		return LightOnFlag;
	case 19:
		Message(1, 0.0);
		Message(14, 0.0);
		break;
	case 20:
		Message(0, 0.0);
		Message(14, 0.0);
		break;
	case 21:
		Message(17, !LightOnFlag);
		return LightOnFlag;
	case 22:
		Message(18, !LightOnFlag);
		return LightOnFlag;
	case 23:
		MessageField = static_cast<int>(floor(value));
		break;
	case -24:
	case -25:
		// FT codes in negative to avoid overlap with 3DPB TLightGroup codes
		render::sprite_set_bitmap(RenderSprite, BmpArr[code == -24]);
		if (UndoOverrideTimer)
			timer::kill(UndoOverrideTimer);
		UndoOverrideTimer = 0;
		if (value > 0)
		{
			TemporaryOverrideFlag = true;
			UndoOverrideTimer = timer::set(value, this, UndoTmpOverride);
		}
		break;
	case -26:
		if (UndoOverrideTimer)
			timer::kill(UndoOverrideTimer);
		UndoOverrideTimer = 0;
		TemporaryOverrideFlag = false;
		render::sprite_set_bitmap(RenderSprite, PreviousBitmap);
		break;
	default:
		break;
	}

	return 0;
}

void TLight::Reset()
{
	if (TimeoutTimer)
		timer::kill(TimeoutTimer);
	if (UndoOverrideTimer)
		timer::kill(UndoOverrideTimer);
	if (FlasherOnFlag)
		flasher_stop(-1);
	TimeoutTimer = 0;
	UndoOverrideTimer = 0;
	LightOnFlag = false;
	LightOnBmpIndex = 0;
	ToggledOffFlag = false;
	ToggledOnFlag = false;
	FlasherOnFlag = false;
	TemporaryOverrideFlag = false;
	TurnOffAfterFlashingFg = false;
	PreviousBitmap = nullptr;
	render::sprite_set_bitmap(RenderSprite, nullptr);
	BmpArr[0] = nullptr;
	if (ListBitmap)
		BmpArr[1] = ListBitmap->at(0);
	MessageField = 0;
}

void TLight::schedule_timeout(float time)
{
	FlashDelay[0] = SourceDelay[0];
	FlashDelay[1] = SourceDelay[1];
	if (TimeoutTimer)
		timer::kill(TimeoutTimer);
	TimeoutTimer = 0;
	if (time > 0.0f)
		TimeoutTimer = timer::set(time, this, TimerExpired);
}

void TLight::TimerExpired(int timerId, void* caller)
{
	auto light = static_cast<TLight*>(caller);
	if (light->FlasherOnFlag)
		light->flasher_stop(-1);
	light->SetSpriteBmp(light->BmpArr[light->LightOnFlag]);
	light->ToggledOffFlag = false;
	light->ToggledOnFlag = false;
	light->FlasherOnFlag = false;
	light->TimeoutTimer = 0;
	if (light->TurnOffAfterFlashingFg)
	{
		light->TurnOffAfterFlashingFg = false;
		light->Message(20, 0.0);
	}
	if (light->Control)
		control::handler(60, light);
}


void TLight::flasher_stop(int bmpIndex)
{
	if (FlashTimer)
		timer::kill(FlashTimer);
	FlashTimer = 0;
	if (bmpIndex >= 0)
	{
		FlashLightOnFlag = bmpIndex;
		SetSpriteBmp(BmpArr[FlashLightOnFlag]);
	}
}

void TLight::flasher_start(bool bmpIndex)
{
	FlashLightOnFlag = bmpIndex;
	flasher_callback(0, this);
}

void TLight::SetSpriteBmp(gdrv_bitmap8* bmp)
{
	PreviousBitmap = bmp;
	if (!TemporaryOverrideFlag)
		render::sprite_set_bitmap(RenderSprite, bmp);
}

void TLight::flasher_callback(int timerId, void* caller)
{
	auto light = static_cast<TLight*>(caller);
	light->FlashLightOnFlag ^= true;
	light->SetSpriteBmp(light->BmpArr[light->FlashLightOnFlag]);
	light->FlashTimer = timer::set(light->FlashDelay[light->FlashLightOnFlag], light, flasher_callback);
}

void TLight::UndoTmpOverride(int timerId, void* caller)
{
	auto light = static_cast<TLight*>(caller);
	light->Message(-26, 0.0f);
}
