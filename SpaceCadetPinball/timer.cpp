#include "pch.h"
#include "timer.h"

#include "pb.h"

int timer::SetCount;
timer_struct* timer::ActiveList;
int timer::MaxCount;
int timer::Count;
timer_struct* timer::FreeList;
timer_struct* timer::TimerBuffer;

int timer::init(int count)
{
	auto buf = new timer_struct[count];
	TimerBuffer = buf;
	if (!buf)
		return 1;
	Count = 0;
	MaxCount = count;
	SetCount = 1;

	for (int index = 0; index < count - 1; index++)
		buf[index].NextTimer = &buf[index + 1];
	buf[count - 1].NextTimer = nullptr;

	ActiveList = nullptr;
	FreeList = buf;
	return 0;
}

void timer::uninit()
{
	delete[] TimerBuffer;
	TimerBuffer = nullptr;
}

int timer::kill(int timerId)
{
	timer_struct* current = ActiveList;
	int index = 0;
	timer_struct* prev = nullptr;
	if (Count <= 0)
		return 0;
	while (timerId != current->TimerId)
	{
		++index;
		prev = current;
		current = current->NextTimer;
		if (index >= Count)
			return 0;
	}
	if (prev)
		prev->NextTimer = current->NextTimer;
	else
		ActiveList = current->NextTimer;

	--Count;
	current->NextTimer = FreeList;
	FreeList = current;
	return timerId;
}

int timer::set(float time, void* caller, void (* callback)(int, void*))
{
	if (Count >= MaxCount)
		return 0;

	auto timer = FreeList;
	FreeList = timer->NextTimer;
	timer->NextTimer = nullptr;

	auto prev = ActiveList;
	auto current = ActiveList;

	auto targetTime = pb::time_ticks + static_cast<int>(time * 1000.0f);
	for (int index = 0; index < Count && targetTime >= current->TargetTime; ++index)
	{
		prev = current;
		current = current->NextTimer;
	}

	if (current != prev)
	{
		timer->NextTimer = prev->NextTimer;
		prev->NextTimer = timer;
	}
	else
	{
		timer->NextTimer = ActiveList;
		ActiveList = timer;
	}

	timer->Caller = caller;
	timer->Callback = callback;
	timer->TimerId = SetCount;
	timer->TargetTime = targetTime;
	Count++;

	SetCount++;
	if (SetCount <= 0)
		SetCount = 1;
	return timer->TimerId;
}

int timer::check()
{
	timer_struct* current = ActiveList;
	int index = 0;
	if (ActiveList)
	{
		while (pb::time_ticks >= current->TargetTime)
		{
			--Count;
			// Advance active list, move current to free
			ActiveList = current->NextTimer;
			current->NextTimer = FreeList;
			FreeList = current;
			if (current->Callback != nullptr)
				current->Callback(current->TimerId, current->Caller);

			current = ActiveList;
			++index;
			if (index > 1)
				break;
			if (!current)
				return index;
		}
		while (current && pb::time_ticks >= current->TargetTime + 100)
		{
			--Count;
			ActiveList = current->NextTimer;
			current->NextTimer = FreeList;
			FreeList = current;
			if (current->Callback != nullptr)
				current->Callback(current->TimerId, current->Caller);

			current = ActiveList;
			++index;
		}
	}
	return index;
}
