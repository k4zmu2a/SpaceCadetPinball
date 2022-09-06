#include "pch.h"
#include "TBlocker.h"


#include "control.h"
#include "loader.h"
#include "render.h"
#include "timer.h"

TBlocker::TBlocker(TPinballTable* table, int groupIndex) : TCollisionComponent(table, groupIndex, true)
{
	visualStruct visual{};

	loader::query_visual(groupIndex, 0, &visual);
	SoundIndex4 = visual.SoundIndex4;
	SoundIndex3 = visual.SoundIndex3;
	InitialDuration = 55;
	ExtendedDuration = 5;
	Threshold = 1000000000.0f;
	Timer = 0;
	MessageField = 0;
	ActiveFlag = 0;
	render::sprite_set_bitmap(RenderSprite, nullptr);
}

int TBlocker::Message(MessageCode code, float value)
{
	switch (code)
	{
	case MessageCode::SetTiltLock:
	case MessageCode::PlayerChanged:
	case MessageCode::Reset:
	case MessageCode::TBlockerDisable:
		if (Timer)
		{
			timer::kill(Timer);
			Timer = 0;
		}
		MessageField = 0;
		ActiveFlag = 0;
		render::sprite_set_bitmap(RenderSprite, nullptr);
		if (code == MessageCode::TBlockerDisable)
			loader::play_sound(SoundIndex3, this, "TBlocker1");
		break;
	case MessageCode::TBlockerEnable:
		ActiveFlag = 1;
		loader::play_sound(SoundIndex4, this, "TBlocker2");
		render::sprite_set_bitmap(RenderSprite, ListBitmap->at(0));
		if (Timer)
			timer::kill(Timer);
		Timer = timer::set(std::max(value, 0.0f), this, TimerExpired);
		break;
	case MessageCode::TBlockerRestartTimeout:
		if (Timer)
			timer::kill(Timer);
		Timer = timer::set(std::max(value, 0.0f), this, TimerExpired);
		break;
	default:
		break;
	}
	
	return 0;
}

void TBlocker::TimerExpired(int timerId, void* caller)
{
	auto blocker = static_cast<TBlocker*>(caller);
	blocker->Timer = 0;
	control::handler(60, blocker);
}
