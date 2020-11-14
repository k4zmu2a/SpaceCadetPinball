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


// Fill memory block with an integer value
inline void memset32(void* ptr, unsigned int value, int count)
{
	auto p = (unsigned int*)ptr;
	for (int i = 0; i < count; i++)
		*p++ = value;
}