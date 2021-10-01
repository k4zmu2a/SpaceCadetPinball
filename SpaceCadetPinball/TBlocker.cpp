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
	TurnOnMsgValue = 55;
	TurnOffMsgValue = 5;
	Threshold = 1000000000.0f;
	Timer = 0;
	MessageField = 0;
	ActiveFlag = 0;
	render::sprite_set_bitmap(RenderSprite, nullptr);
}

int TBlocker::Message(int code, float value)
{
	switch (code)
	{
	case 1011:
	case 1020:
	case 1024:
	case 51:
		if (Timer)
		{
			timer::kill(Timer);
			Timer = 0;
		}
		MessageField = 0;
		ActiveFlag = 0;
		render::sprite_set_bitmap(RenderSprite, nullptr);
		if (code == 51)
			loader::play_sound(SoundIndex3);
		return 0;
	case 52:
		ActiveFlag = 1;
		loader::play_sound(SoundIndex4);
		render::sprite_set_bitmap(RenderSprite, ListBitmap->at(0));
		break;
	case 59:
		break;
	default:
		return 0;
	}
	if (Timer)
		timer::kill(Timer);

	float timerTime;
	if (value <= 0.0f)
		timerTime = 0.0;
	else
		timerTime = value;
	Timer = timer::set(timerTime, this, TimerExpired);
	return 0;
}

void TBlocker::TimerExpired(int timerId, void* caller)
{
	auto blocker = static_cast<TBlocker*>(caller);
	blocker->Timer = 0;
	control::handler(60, blocker);
}
