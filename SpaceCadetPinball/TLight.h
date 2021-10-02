#pragma once
#include "TPinballComponent.h"

struct gdrv_bitmap8;

struct flasher_type
{
	render_sprite_type_struct* Sprite;
	gdrv_bitmap8* BmpArr[2];
	int Unknown3;
	int Unknown4;
	float TimerDelay[2];
	int Timer;
	int BmpIndex;
};


struct TLight_player_backup
{
	int MessageField;
	int BmpIndex1;
	int FlasherActive;
	int Unknown3;
	int Unknown4;
	int BmpIndex2;
};


class TLight :
	public TPinballComponent
{
public:
	TLight(TPinballTable* table, int groupIndex);
	int Message(int code, float value) override;
	void Reset();
	void schedule_timeout(float time);

	static void TimerExpired(int timerId, void* caller);
	static void flasher_stop(flasher_type* flash, int bmpIndex);
	static void flasher_start(struct flasher_type* flash, int bmpIndex);	
	static void flasher_callback(int timerId, void* caller);

	flasher_type Flasher{};
	int BmpIndex1{};
	int FlasherActive;
	int FlasherFlag1{};
	int FlasherFlag2{};
	int TurnOffAfterFlashingFg{};
	int BmpIndex2{};
	float FlasherDelay[2]{};
	int Timer1;
	int Timer2;
	float Unknown20F{};
	TLight_player_backup PlayerData[4]{};
};
