#include "pch.h"
#include "TSound.h"

#include "loader.h"

TSound::TSound(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, true)
{
	visualStruct visual{};
	loader::query_visual(groupIndex, 0, &visual);
	this->SoundIndex = visual.SoundIndex4;
}

float TSound::Play()
{
	return loader::play_sound(this->SoundIndex);
}
