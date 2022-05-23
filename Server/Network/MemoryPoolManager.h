#pragma once
#include "MemoryPool.h"
#include <vector>

template<typename T>
class MemoryPoolManager
{
public:
	static MemoryPoolManager& GetInstance(size_t poolCount = 0, size_t poolSize = 0)
	{
		static MemoryPoolManager instance(poolCount, poolSize);
		return instance;
	}

private:
	MemoryPoolManager(size_t poolCount, size_t poolSize)
		: mMemPools{ poolCount }
	{
		assert(poolSize > 0);
		for (int i = 0; i < mMemPools.size(); i++)
			mMemPools[i].Init(poolSize);
	}
	MemoryPoolManager(const MemoryPoolManager&) = delete;
	MemoryPoolManager& operator=(const MemoryPoolManager&) = delete;
	MemoryPoolManager(MemoryPoolManager&&) = delete;
	MemoryPoolManager& operator=(MemoryPoolManager&&) = delete;

public:
	void* Allocate(int idx)
	{		
		assert(0 <= idx && idx < mMemPools.size());
		void* ptr = mMemPools[idx].Alloc();
		mMemIdxCache[ptr] = idx;
		return ptr;
	}

	void Deallocate(void* ptr)
	{
		int idx = mMemIdxCache[ptr];
		assert(0 <= idx && idx < mMemPools.size());
		mMemIdxCache[ptr] = -1;
		mMemPools[idx].Dealloc(ptr);
	}

private:
	std::unordered_map<void*, int> mMemIdxCache;
	std::vector<MemoryPool<T>> mMemPools;
};