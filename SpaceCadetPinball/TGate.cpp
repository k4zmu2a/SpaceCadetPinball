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
	control::handler(MessageCode::Reset, this);
}

int TGate::Message(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::TGateDisable:
		ActiveFlag = 0;
		render::sprite_set_bitmap(RenderSprite, nullptr);
		loader::play_sound(SoundIndex3, this, "TGate1");
		break;
	case MessageCode::Reset:
	case MessageCode::TGateEnable:
		ActiveFlag = 1;
		render::sprite_set_bitmap(RenderSprite, ListBitmap->at(0));
		if (code == MessageCode::TGateEnable)
			loader::play_sound(SoundIndex4, this, "TGate2");
		break;
	default: break;
	}

	control::handler(code, this);
	return 0;
}
