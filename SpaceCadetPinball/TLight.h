#pragma once
#include "TPinballComponent.h"

struct gdrv_bitmap8;

struct TLight_player_backup
{
	int MessageField;
	bool LightOnFlag;
	int LightOnBmpIndex;
	bool FlasherOnFlag;
};


class TLight :
	public TPinballComponent2
{
public:
	TLight(TPinballTable* table, int groupIndex);
	int Message2(MessageCode code, float value) override;
	void Reset();
	void schedule_timeout(float time);
	void flasher_stop(int bmpIndex);
	void flasher_start(bool bmpIndex);
	void SetSpriteBmp(gdrv_bitmap8* bmp);

	static void TimerExpired(int timerId, void* caller);
	static void flasher_callback(int timerId, void* caller);
	static void UndoTmpOverride(int timerId, void* caller);

	gdrv_bitmap8* BmpArr[2];
	float FlashDelay[2];
	int FlashTimer;
	bool FlashLightOnFlag;
	bool LightOnFlag{};
	bool FlasherOnFlag;
	bool ToggledOffFlag{};
	bool ToggledOnFlag{};
	bool TurnOffAfterFlashingFg{};
	int LightOnBmpIndex{};
	float SourceDelay[2]{};
	int TimeoutTimer;
	int UndoOverrideTimer;
	bool TemporaryOverrideFlag{};
	gdrv_bitmap8* PreviousBitmap{};
	TLight_player_backup PlayerData[4]{};
};
