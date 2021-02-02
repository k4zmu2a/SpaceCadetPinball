#include "pch.h"
#include "memory.h"

size_t memory::use_total;
int memory::critical_allocation;
void (*memory::critical_callback)();
std::map<void*, size_t> memory::alloc_map{};

void memory::init(void (*callback)())
{
	critical_callback = callback;
}

char* memory::allocate(size_t size)
{
	auto buf = static_cast<char*>(malloc(size));
	if (!buf)
	{
		if (critical_allocation && critical_callback)
			critical_callback();
		return nullptr;
	}

	use_total += size;
	alloc_map[buf] = size;
	return buf;
}

void memory::free(void* buf)
{
	auto alloc = alloc_map.find(buf);
	if (alloc == alloc_map.end())
	{
		assertm(false, "Unknown memory type");
		return;
	}

	use_total -= alloc->second;
	std::free(alloc->first);
}
