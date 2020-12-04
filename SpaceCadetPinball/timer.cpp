#include "pch.h"
#include "timer.h"

#include "memory.h"
#include "pb.h"

timer_struct timer::timerStruct{};
int timer::set_count;

int timer::init(int count)
{
	auto buf = (timer_sub_struct*)memory::allocate(sizeof(timer_sub_struct) * count);
	timerStruct.TimerMem = buf;
	if (!buf)
		return 1;
	timerStruct.Count = 0;
	timerStruct.MaxCount = count;
	set_count = 1;

	for (int index = 0; index < count - 1; index++)
		buf[index].NextTimer = &buf[index + 1];
	buf[count - 1].NextTimer = nullptr;

	timerStruct.NextTimer = nullptr;
	timerStruct.LastTimer = buf;
	return 0;
}

void timer::uninit()
{
	if (timerStruct.TimerMem)
		memory::free(timerStruct.TimerMem);
	timerStruct.TimerMem = nullptr;
}

int timer::kill(int timerId)
{
	timer_sub_struct* next = timerStruct.NextTimer;
	int index = 0;
	timer_sub_struct* current = nullptr;
	if (timerStruct.Count <= 0)
		return 0;
	while (timerId != next->TimerId)
	{
		++index;
		current = next;
		next = next->NextTimer;
		if (index >= timerStruct.Count)
			return 0;
	}
	if (current)
		current->NextTimer = next->NextTimer;
	else
		timerStruct.NextTimer = next->NextTimer;

	--timerStruct.Count;
	next->NextTimer = timerStruct.LastTimer;
	timerStruct.LastTimer = next;
	return timerId;
}

int timer::set(float time, void* caller, void (* callback)(int, void*))
{
	if (timerStruct.Count >= timerStruct.MaxCount)
		return 0;

	/*timerStruct.LastTimer->NextTimer = nullptr;
	timerStruct.LastTimer = timerStruct.LastTimer->NextTimer;*/

	auto lastNext = timerStruct.LastTimer->NextTimer;
	timerStruct.LastTimer->NextTimer = nullptr;
	timerStruct.LastTimer = lastNext;

	auto prev = timerStruct.NextTimer;
	auto current = timerStruct.NextTimer;

	auto targetTime = pb::time_ticks + static_cast<int>(time * 1000.0f);
	for (int index = 0; index < timerStruct.Count && targetTime >= current->TargetTime; ++index)
	{
		prev = current;
		current = current->NextTimer;
	}

	auto last = timerStruct.LastTimer;
	if (current != prev)
	{
		timerStruct.LastTimer->NextTimer = prev->NextTimer;
		prev->NextTimer = last;
	}
	else
	{
		timerStruct.LastTimer->NextTimer = timerStruct.NextTimer;
		timerStruct.NextTimer = last;
	}

	last->Caller = caller;
	last->Callback = callback;
	last->TimerId = set_count;
	last->TargetTime = targetTime;
	timerStruct.Count++;

	set_count++;
	if (set_count <= 0)
		set_count = 1;
	return last->TimerId;
}

int timer::check()
{
	timer_sub_struct curCopy{};
	timer_sub_struct* current = timerStruct.NextTimer;
	int index = 0;
	if (timerStruct.NextTimer)
	{
		while (pb::time_ticks >= current->TargetTime)
		{
			--timerStruct.Count;
			memcpy(&curCopy, current, sizeof curCopy);
			timer_sub_struct** nextPtr = &current->NextTimer;
			current = current->NextTimer;		
			timerStruct.NextTimer = current;
			*nextPtr = timerStruct.LastTimer;
			timerStruct.LastTimer = current;
			if (curCopy.Callback != nullptr)
			{
				curCopy.Callback(curCopy.TimerId, curCopy.Caller);
				current = timerStruct.NextTimer;
			}
			++index;
			if (index > 1)
				break;
			if (!current)
				return index;
		}
		while (current && pb::time_ticks >= current->TargetTime + 100)
		{
			--timerStruct.Count;
			memcpy(&curCopy, current, sizeof curCopy);
			timer_sub_struct** nextPtr = &current->NextTimer;		
			current = current->NextTimer;
			timerStruct.NextTimer = current;
			*nextPtr = timerStruct.LastTimer;
			timerStruct.LastTimer = current;
			if (curCopy.Callback != nullptr)
			{
				curCopy.Callback(curCopy.TimerId, curCopy.Caller);
				current = timerStruct.NextTimer;
			}
			++index;
		}
	}
	return index;
}
