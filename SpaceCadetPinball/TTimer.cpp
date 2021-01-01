#include "pch.h"
#include "TTimer.h"

#include "control.h"
#include "timer.h"

TTimer::TTimer(TPinballTable* table, int groupIndex) : TPinballComponent(table, groupIndex, true)
{
	Timer = 0;
}

int TTimer::Message(int code, float value)
{
	if (code == 59)
	{
		if (Timer)
			timer::kill(Timer);
		Timer = timer::set(value, this, TimerExpired);
	}
	else if (code == 1011 || code == 1022 || code == 1024)
	{
		if (Timer)
		{
			timer::kill(Timer);
			Timer = 0;
		}
	}
	return 0;
}


void TTimer::TimerExpired(int timerId, void* caller)
{
	auto timer = static_cast<TTimer*>(caller);
	timer->Timer = 0;
	control::handler(60, timer);
}
