#include "stdafx.h"
#include <queue>

class Interpolator
{
private:
	using TimeStamp = Clock::time_point;

public:
	struct Entry
	{
		TimeStamp timeStamp;

		XMFLOAT3 position;
		XMFLOAT4 rotation;

		Entry(const XMFLOAT3& pos, const XMFLOAT4& rot)
		{
			timeStamp = Clock::now();
			position = pos;
			rotation = rot;
		}
	};

public:
	Interpolator()
	{

	}

	void Enqueue(const XMFLOAT3& pos, const XMFLOAT4& rot)
	{
		mEntryQueueMut.lock();
		mEntryQueue.emplace(pos, rot);
		mEntryQueueMut.unlock();
	}

	void Interpolate(float dt, XMFLOAT3& targetPos, XMFLOAT4& targetRot)
	{
		if (IsEmpty()) return;
		

	}

private:
	bool IsEmpty()
	{
		mEntryQueueMut.lock();
		bool res = mEntryQueue.empty();
		mEntryQueueMut.unlock();
		return res;
	}

private:
	std::queue<Entry> mEntryQueue;
	std::mutex mEntryQueueMut;
};