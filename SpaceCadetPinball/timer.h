#pragma once

struct timer_struct
{
	int TargetTime;
	void* Caller;
	void (* Callback)(int, void*);
	timer_struct* NextTimer;
	int TimerId;
};

class timer
{
public:
	static int init(int count);
	static void uninit();
	static int kill(int timerId);
	static int set(float time, void* caller, void (* callback)(int, void*));
	static int check();

private:
	static int SetCount;
	static timer_struct* ActiveList;
	static int MaxCount;
	static int Count;
	static timer_struct* FreeList;
	static timer_struct* TimerBuffer;
};
