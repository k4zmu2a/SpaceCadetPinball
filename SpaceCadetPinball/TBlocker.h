#pragma once
#include "TCollisionComponent.h"

class TBlocker :
	public TCollisionComponent2
{
public:
	TBlocker(TPinballTable* table, int groupIndex);
	int Message2(MessageCode code, float value) override;

	static void TimerExpired(int timerId, void* caller);

	int InitialDuration;
	int ExtendedDuration;
	int Timer;
	int SoundIndex4;
	int SoundIndex3;
};
