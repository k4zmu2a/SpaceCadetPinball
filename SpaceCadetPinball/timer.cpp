#include "pch.h"
#include "timer.h"

#include "memory.h"

timer_struct timer::timer_struct;
int timer::setCount;

int timer::init(int count)
{
	char* buf; // eax
	int index; // edx
	int* v4; // ecx

	buf = memory::allocate(20 * count);
	timer_struct.buffer1 = buf;
	if (!buf)
		return 1;
	timer_struct.target = 0;
	index = count - 1;
	timer_struct.count = count;
	setCount = 1;
	if (count - 1 > 0)
	{
		v4 = (int*)(buf + 12);
		do
		{
			*v4 = (int)(v4 + 2);
			v4 += 5;
			--index;
		}
		while (index);
	}
	*(int*)&buf[20 * count - 8] = 0;
	timer_struct.target2 = 0;
	timer_struct.buffer2 = buf;
	return 0;
}
