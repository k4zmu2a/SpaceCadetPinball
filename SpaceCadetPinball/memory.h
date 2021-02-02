#pragma once
#include <map>

class memory
{
public:
	static void init(void (*callback)(void));
	static char* allocate(size_t size);
	static void free(void* buf);

	template <typename T>
	static T* allocate(size_t count = 1, size_t add = 0)
	{
		size_t size = sizeof(T) * count + add;
		auto buf = static_cast<T*>(malloc(size));
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

	template <typename T>
	static T* realloc(T* buf, size_t size)
	{
		if (!buf)
			return reinterpret_cast<T*>(allocate(size));

		auto alloc = alloc_map.find(buf);
		if (alloc == alloc_map.end())
		{
			assertm(false, "Unknown memory type");
			return buf;
		}

		auto newBuf = static_cast<T*>(std::realloc(alloc->first, size));
		if (!newBuf)
		{
			if (critical_allocation && critical_callback)
				critical_callback();
			return nullptr;
		}

		use_total += size - alloc->second;
		alloc_map.erase(alloc);
		alloc_map[newBuf] = size;
		return newBuf;
	}

	static size_t use_total;
	static int critical_allocation;
private:
	static void (*critical_callback)();
	static std::map<void*, size_t> alloc_map;
};
