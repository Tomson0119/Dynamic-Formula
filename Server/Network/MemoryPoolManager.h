#pragma once
#include "MemoryPool.h"
#include <vector>
#include <cassert>

template<typename T>
class MemoryPoolManager
{
public:
	static MemoryPoolManager& GetInstance(size_t poolSize = 0)
	{
		static MemoryPoolManager instance(poolSize);
		return instance;
	}

private:
	MemoryPoolManager(size_t poolSize)
	{
		assert(poolSize > 0);
		mMemPool.Init(poolSize);
	}
	MemoryPoolManager(const MemoryPoolManager&) = delete;
	MemoryPoolManager& operator=(const MemoryPoolManager&) = delete;
	MemoryPoolManager(MemoryPoolManager&&) = delete;
	MemoryPoolManager& operator=(MemoryPoolManager&&) = delete;

public:
	void* Allocate()
	{
		void* ptr = mMemPool.Alloc();
		return ptr;
	}

	void Deallocate(void* ptr)
	{
		mMemPool.Dealloc(ptr);
	}

private:
	MemoryPool<T> mMemPool;
};