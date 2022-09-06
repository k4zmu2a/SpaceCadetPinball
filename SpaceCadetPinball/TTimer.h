#pragma once
#include "TPinballComponent.h"

class TTimer :
	public TPinballComponent2
{
public:
	TTimer(TPinballTable* table, int groupIndex);
	int Message2(MessageCode code, float value) override;
	static void TimerExpired(int timerId, void* caller);

	int Timer;
};
