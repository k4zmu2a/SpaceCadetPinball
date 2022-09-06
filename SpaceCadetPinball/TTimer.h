#pragma once
#include "TPinballComponent.h"

class TTimer :
	public TPinballComponent
{
public:
	TTimer(TPinballTable* table, int groupIndex);
	int Message(MessageCode code, float value) override;
	static void TimerExpired(int timerId, void* caller);

	int Timer;
};
