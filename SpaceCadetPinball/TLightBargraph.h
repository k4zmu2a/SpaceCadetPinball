#pragma once
#include "TLightGroup.h"

class TLightBargraph :
	public TLightGroup
{
public:
	TLightBargraph(TPinballTable* table, int groupIndex);
	~TLightBargraph() override;
	int Message(int code, float value) override;
	void Reset() override;

	static void BargraphTimerExpired(int timerId, void* caller);

	float* TimerTimeArray;
	int TimerBargraph{};
	int TimeIndex{};
	int PlayerTimerIndexBackup[4]{};
};
