#pragma once

struct __declspec(align(4)) timer_struct
{
	int target2;
	int count;
	int target;
	char* buffer2;
	char* buffer1;
};


class timer
{
public:
	static int init(int count);

private:
	static timer_struct timer_struct;
	static int setCount;
};
