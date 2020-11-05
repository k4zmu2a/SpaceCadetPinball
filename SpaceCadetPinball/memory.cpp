#include "pch.h"
#include "memory.h"

unsigned int memory::use_total;
int memory::critical_allocation;
void (*memory::critical_callback)();

void memory::init(void (*callback)())
{
	critical_callback = callback;
}

char* memory::allocate(unsigned int size)
{
	char* buf = static_cast<char*>(malloc(size + 4));
	if (buf)
	{
		*(unsigned int*)buf = size << 8;
		use_total += size + 4;
		*buf = size >= 0xFFDC ? -91 : 90;
		return buf + 4;
	}
	if (critical_allocation && critical_callback)
		critical_callback();
	return nullptr;
}

void memory::free(void* buf)
{
	unsigned int* bufStart = static_cast<unsigned int*>(buf) - 1;
	use_total -= (*bufStart >> 8) + 4;
	char firstChar = *(char*)bufStart;
	if (firstChar == 90 || firstChar == -91)
		std::free(bufStart);
	else
		assertm(false, "Unknown memory type");
}

char* memory::realloc(void* buf, unsigned int size)
{
	if (!buf)
		return allocate(size);

	char* bufStart = static_cast<char*>(buf) - 4;
	use_total -= *(unsigned int*)bufStart >> 8;
	if (*bufStart != 90 && *bufStart != -91 ||
		(bufStart = static_cast<char*>(std::realloc(bufStart, size + 4))) != nullptr)
	{
		char bufType = *bufStart;
		*(unsigned int*)bufStart = size << 8;
		use_total += size;
		*bufStart = bufType;
		return bufStart + 4;
	}
	if (critical_allocation && critical_callback)
		critical_callback();
	return nullptr;
}
