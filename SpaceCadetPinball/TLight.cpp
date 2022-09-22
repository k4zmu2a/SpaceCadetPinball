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

int TLight::Message(MessageCode code, float value)
{
	int bmpIndex;

	switch (code)
	{
	case MessageCode::Reset:
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
	case MessageCode::PlayerChanged:
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
				Message(MessageCode::TLightSetOnStateBmpIndex, static_cast<float>(LightOnBmpIndex));
			}
			if (LightOnFlag)
				Message(MessageCode::TLightTurnOn, 0.0);
			if (FlasherOnFlag)
				Message(MessageCode::TLightFlasherStart, 0.0);
			break;
		}
	case MessageCode::TLightTurnOff:
		LightOnFlag = false;
		if (!FlasherOnFlag && !ToggledOffFlag && !ToggledOnFlag)
			SetSpriteBmp(BmpArr[0]);
		break;
	case MessageCode::TLightTurnOn:
		LightOnFlag = true;
		if (!FlasherOnFlag && !ToggledOffFlag && !ToggledOnFlag)
			SetSpriteBmp(BmpArr[1]);
		break;
	case MessageCode::TLightGetLightOnFlag:
		return LightOnFlag;
	case MessageCode::TLightGetFlasherOnFlag:
		return FlasherOnFlag;
	case MessageCode::TLightFlasherStart:
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
	case MessageCode::TLightApplyMultDelay:
		FlashDelay[0] = value * SourceDelay[0];
		FlashDelay[1] = value * SourceDelay[1];
		break;
	case MessageCode::TLightApplyDelay:
		FlashDelay[0] = SourceDelay[0];
		FlashDelay[1] = SourceDelay[1];
		break;
	case MessageCode::TLightFlasherStartTimed:
		if (!FlasherOnFlag)
			flasher_start(LightOnFlag);
		FlasherOnFlag = true;
		ToggledOnFlag = false;
		TurnOffAfterFlashingFg = false;
		ToggledOffFlag = false;
		schedule_timeout(value);
		break;
	case MessageCode::TLightTurnOffTimed:
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
	case MessageCode::TLightTurnOnTimed:
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
	case MessageCode::TLightSetOnStateBmpIndex:
		LightOnBmpIndex = Clamp(static_cast<int>(floor(value)), 0, static_cast<int>(ListBitmap->size()) - 1);
		BmpArr[0] = -1;
		BmpArr[1] = LightOnBmpIndex;
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
	case MessageCode::TLightIncOnStateBmpIndex:
		bmpIndex = LightOnBmpIndex + 1;
		if (bmpIndex >= static_cast<int>(ListBitmap->size()))
			bmpIndex = static_cast<int>(ListBitmap->size()) - 1;
		Message(MessageCode::TLightSetOnStateBmpIndex, static_cast<float>(bmpIndex));
		break;
	case MessageCode::TLightDecOnStateBmpIndex:
		bmpIndex = LightOnBmpIndex - 1;
		if (bmpIndex < 0)
			bmpIndex = 0;
		Message(MessageCode::TLightSetOnStateBmpIndex, static_cast<float>(bmpIndex));
		break;
	case MessageCode::TLightResetTimed:
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
	case MessageCode::TLightFlasherStartTimedThenStayOn:
		TurnOffAfterFlashingFg = false;
		if (UndoOverrideTimer)
			timer::kill(UndoOverrideTimer);
		UndoOverrideTimer = 0;
		Message(MessageCode::TLightTurnOn, 0.0);
		Message(MessageCode::TLightFlasherStartTimed, value);
		break;
	case MessageCode::TLightFlasherStartTimedThenStayOff:
		if (UndoOverrideTimer)
			timer::kill(UndoOverrideTimer);
		UndoOverrideTimer = 0;
		Message(MessageCode::TLightFlasherStartTimed, value);
		TurnOffAfterFlashingFg = true;
		break;
	case MessageCode::TLightToggleValue:
		Message(static_cast<int>(floor(value)) ? MessageCode::TLightTurnOn : MessageCode::TLightTurnOff, 0.0);
		return LightOnFlag;
	case MessageCode::TLightResetAndToggleValue:
		Message(MessageCode::TLightToggleValue, value);
		Message(MessageCode::TLightResetTimed, 0.0);
		return LightOnFlag;
	case MessageCode::TLightResetAndTurnOn:
		Message(MessageCode::TLightTurnOn, 0.0);
		Message(MessageCode::TLightResetTimed, 0.0);
		break;
	case MessageCode::TLightResetAndTurnOff:
		Message(MessageCode::TLightTurnOff, 0.0);
		Message(MessageCode::TLightResetTimed, 0.0);
		break;
	case MessageCode::TLightToggle:
		Message(MessageCode::TLightToggleValue, !LightOnFlag);
		return LightOnFlag;
	case MessageCode::TLightResetAndToggle:
		Message(MessageCode::TLightResetAndToggleValue, !LightOnFlag);
		return LightOnFlag;
	case MessageCode::TLightSetMessageField:
		MessageField = static_cast<int>(floor(value));
		break;
	case MessageCode::TLightFtTmpOverrideOn:
	case MessageCode::TLightFtTmpOverrideOff:
		// FT codes in negative to avoid overlap with 3DPB TLightGroup codes
		SpriteSet(BmpArr[code == MessageCode::TLightFtTmpOverrideOn]);
		if (UndoOverrideTimer)
			timer::kill(UndoOverrideTimer);
		UndoOverrideTimer = 0;
		if (value > 0)
		{
			TemporaryOverrideFlag = true;
			UndoOverrideTimer = timer::set(value, this, UndoTmpOverride);
		}
		break;
	case MessageCode::TLightFtResetOverride:
		if (UndoOverrideTimer)
			timer::kill(UndoOverrideTimer);
		UndoOverrideTimer = 0;
		TemporaryOverrideFlag = false;
		SpriteSet(PreviousBitmap);
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
	PreviousBitmap = -1;
	BmpArr[0] = -1;
	BmpArr[1] = 0;
	SetSpriteBmp(BmpArr[0]);
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
		light->Message(MessageCode::TLightResetAndTurnOff, 0.0);
	}
	if (light->Control)
		control::handler(MessageCode::ControlTimerExpired, light);
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

void TLight::SetSpriteBmp(int index)
{
	PreviousBitmap = index;
	if (!TemporaryOverrideFlag)
		SpriteSet(index);
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
	light->Message(MessageCode::TLightFtResetOverride, 0.0f);
}

bool TLight::light_on() const
{
	return LightOnFlag || ToggledOnFlag || FlasherOnFlag;
}
