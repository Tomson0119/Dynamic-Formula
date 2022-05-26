#pragma once
#include "MemoryPool.h"
#include <vector>
#include <cassert>

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
		return ptr;
	}

	void Deallocate(void* ptr)
	{
		for (int i = 0; i < mMemPools.size() - 1; i++)
		{
			void* curr = mMemPools[i].GetMemStartAddr();
			void* next = mMemPools[i + 1].GetMemStartAddr();
			if (curr <= ptr && ptr < next)
			{
				mMemPools[i].Dealloc(ptr);
				return;
			}
		}
		mMemPools.back().Dealloc(ptr);
	}

private:
	std::vector<MemoryPool<T>> mMemPools;
};