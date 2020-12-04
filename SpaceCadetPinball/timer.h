#pragma once

struct __declspec(align(4)) timer_sub_struct
{
	int TargetTime;
	void* Caller;
	void (* Callback)(int, void*);
	timer_sub_struct* NextTimer;
	int TimerId;
};

struct __declspec(align(4)) timer_struct
{
	timer_sub_struct* NextTimer;
	int MaxCount;
	int Count;
	timer_sub_struct* LastTimer;
	timer_sub_struct* TimerMem;
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
	static timer_struct timerStruct;
	static int set_count;
};
