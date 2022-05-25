#pragma once
#include <list>

template <typename T>
class MemoryPool
{
public:
	MemoryPool() = default;
	MemoryPool(const MemoryPool&) = delete;
	MemoryPool& operator=(const MemoryPool&) = delete;
	MemoryPool(MemoryPool&&) = delete;
	MemoryPool& operator=(MemoryPool&&) = delete;

	~MemoryPool()
	{
		if (mPool) delete[] mPool;
	}

	void Init(size_t count)
	{
		mBlockCount = count;
		mPoolSize = count * sizeof(T);
		mPool = new T[count];

		for (int i = 0; i < (int)mBlockCount; i++)
		{
			std::byte* ptr = reinterpret_cast<std::byte*>(mPool + i);
			mMemAddrs.push_front(ptr);
		}
	}

	void* Alloc()
	{
		if (mMemAddrs.empty())
			return nullptr;

		auto ptr = mMemAddrs.front();
		mMemAddrs.pop_front();

		return reinterpret_cast<void*>(ptr);
	}

	void Dealloc(void* p)
	{
		if (p)
		{
			auto ptr = reinterpret_cast<std::byte*>(p);
			p = nullptr;
			mMemAddrs.push_front(ptr);
		}
	}

	size_t GetTotalSize()
	{
		return mPoolSize;
	}

	size_t GetAllocatedCount()
	{
		return mBlockCount - mMemAddrs.size();
	}

private:
	T* mPool = nullptr;
	std::list<std::byte*> mMemAddrs;

	size_t mBlockCount = 0;
	size_t mPoolSize = 0;
};