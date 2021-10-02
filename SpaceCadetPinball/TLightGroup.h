#pragma once
#include "TPinballComponent.h"


class TLight;

struct TLightGroup_player_backup
{
	int MessageField;
	int MessageField2;
	float Timer1Time;
	int Unknown3;
};


class TLightGroup :
	public TPinballComponent
{
public:
	TLightGroup(TPinballTable* table, int groupIndex);
	~TLightGroup() override = default;
	int Message(int code, float value) override;
	virtual void Reset();
	void reschedule_animation(float time);
	void start_animation();
	int next_light_up();
	int next_light_down();

	static void TimerExpired(int timerId, void* caller);
	static void NotifyTimerExpired(int timerId, void* caller);

	std::vector<TLight*> List;
	float Timer1Time{};
	float Timer1TimeDefault;
	int MessageField2{};
	int AnimationFlag{};
	int NotifyTimer;
	int Timer;
	TLightGroup_player_backup PlayerData[4]{};
};
