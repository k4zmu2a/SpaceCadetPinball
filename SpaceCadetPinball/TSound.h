#pragma once
#include "TPinballComponent.h"

class TSound :
	public TPinballComponent
{
public:
	TSound(TPinballTable* table, int groupIndex);
	float Play();
	float Play(TPinballComponent *soundSource, const char* info);

	int SoundIndex;
};
