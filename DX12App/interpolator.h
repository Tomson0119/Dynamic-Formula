#include "stdafx.h"
#include <queue>

class Interpolator
{
public:
	struct Entry
	{
		bool empty;
		Clock::time_point timeStamp;

		XMFLOAT3 position;
		XMFLOAT4 rotation;

		Entry()
			: empty{ true },
			  position{},
			  rotation{}
		{
		}

		Entry(const XMFLOAT3& pos, const XMFLOAT4& rot)
			: empty{ false } 
		{
			timeStamp = Clock::now();
			position = pos;
			rotation = rot;
		}
	};

public:
	Interpolator()
		: mProgress{ 0.0f } 
	{
	}

	void Enqueue(const XMFLOAT3& pos, const XMFLOAT4& rot)
	{
		mEntryQueueMut.lock();
		mEntryQueue.push(Entry(pos, rot));
		mEntryQueueMut.unlock();
	}

	void Interpolate(float dt, XMFLOAT3& targetPos, XMFLOAT4& targetRot)
	{
		mEntryQueueMut.lock();
		if (mEntryQueue.empty())
		{
			mEntryQueueMut.unlock();
			return;
		}

		Entry next = mEntryQueue.front();
		if (mPrevEntry.empty)
		{
			mEntryQueue.pop();
			mEntryQueueMut.unlock();

			mPrevEntry = next;
			targetPos = next.position;
			targetRot = next.rotation;
			return;
		}

		mEntryQueueMut.unlock();

		mProgress += dt;
		float timeBetween = GetDurationSec(next.timeStamp, mPrevEntry.timeStamp);
		float progress = std::min(1.0f, mProgress / timeBetween);

		targetPos = Vector3::Lerp(mPrevEntry.position, next.position, progress);
		targetRot = Vector4::Slerp(mPrevEntry.rotation, next.rotation, progress);

		if (progress >= 1.0f)
		{
			mEntryQueueMut.lock();
			mEntryQueue.pop();
			mEntryQueueMut.unlock();

			mPrevEntry = next;
			mProgress -= timeBetween;
			Interpolate(0.0f, targetPos, targetRot);
		}
	}

	void Clear()
	{
		mEntryQueueMut.lock();
		std::queue<Entry> temp;
		std::swap(mEntryQueue, temp);
		mEntryQueueMut.unlock();

		mProgress = 0.0f;
		mPrevEntry = Entry{};
	}

private:
	static float GetDurationSec(Clock::time_point& a, Clock::time_point& b)
	{
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>
			(a - b).count();
		return (float)msec / 1000.0f;
	}

private:
	std::queue<Entry> mEntryQueue;
	std::mutex mEntryQueueMut;

	Entry mPrevEntry;
	float mProgress;
};