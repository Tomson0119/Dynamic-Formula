#pragma once
#include <queue>
#include <concurrent_queue.h>

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
			void* ptr = reinterpret_cast<void*>(mPool + i);
			mMemAddrs.push(ptr);
		}
	}

	void* Alloc()
	{
		std::unique_lock<std::mutex> lock{ mQueueMut };
		if (mMemAddrs.empty())
			return nullptr;

		//void* ptr = nullptr;
		//while (mMemAddrs.try_pop(ptr) == false);

		void* ptr = mMemAddrs.front();
		mMemAddrs.pop();
		return ptr;
	}

	void Dealloc(void* p)
	{
		if (p)
		{
			std::unique_lock<std::mutex> lock{ mQueueMut };
			auto ptr = reinterpret_cast<void*>(p);
			p = nullptr;
			mMemAddrs.push(ptr);
		}
	}

	size_t GetTotalSize()
	{
		return mPoolSize;
	}

private:
	T* mPool = nullptr;
	//concurrency::concurrent_queue<void*> mMemAddrs;
	std::queue<void*> mMemAddrs;
	std::mutex mQueueMut;

	size_t mBlockCount = 0;
	std::atomic_size_t mPoolSize = 0;
};