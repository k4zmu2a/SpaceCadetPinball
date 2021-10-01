#include "pch.h"
#include "TGate.h"


#include "control.h"
#include "loader.h"
#include "render.h"

TGate::TGate(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	visualStruct visual{};

	loader::query_visual(groupIndex, 0, &visual);
	SoundIndex4 = visual.SoundIndex4;
	SoundIndex3 = visual.SoundIndex3;
	ActiveFlag = 1;
	render::sprite_set_bitmap(RenderSprite, ListBitmap->at(0));
	control::handler(1024, this);
}

int TGate::Message(int code, float value)
{
	if (code != 1020)
	{
		if (code == 53)
		{
			ActiveFlag = 0;
			render::sprite_set_bitmap(RenderSprite, nullptr);
			loader::play_sound(SoundIndex3);
		}
		else if (code == 54 || code == 1024)
		{
			ActiveFlag = 1;
			render::sprite_set_bitmap(RenderSprite, ListBitmap->at(0));
			if (code == 54)
				loader::play_sound(SoundIndex4);
		}
		control::handler(code, this);
	}
	return 0;
}
