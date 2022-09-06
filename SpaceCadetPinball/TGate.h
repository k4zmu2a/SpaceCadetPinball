#pragma once
#include "TCollisionComponent.h"

class TGate :
	public TCollisionComponent2
{
public:
	TGate(TPinballTable* table, int groupIndex);
	int Message2(MessageCode code, float value) override;

	int SoundIndex4;
	int SoundIndex3;
};
