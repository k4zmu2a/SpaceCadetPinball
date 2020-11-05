#pragma once
class memory
{
public:
	static void init(void (*callback)(void));
	static char* allocate(unsigned int size);
	static void free(void* buf);
	static char* realloc(void* buf, unsigned int size);

	static unsigned int use_total;
	static int critical_allocation;
	static void (*critical_callback)();
};
