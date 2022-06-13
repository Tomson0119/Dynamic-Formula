#include "stdafx.h"

class Interpolator
{
public:
	struct Entry
	{
		bool empty;
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
			position = pos;
			rotation = rot;
		}
	};

public:
	Interpolator()
		: mCurrentTime{ 0 }, mPrevTimePoint{ 0 }
	{
	}

	void Enqueue(uint64_t timePoint, const XMFLOAT3& pos, const XMFLOAT4& rot)
	{
		mEntryMapMut.lock();
		mEntries.insert({ timePoint, Entry(pos, rot) });
		mEntryMapMut.unlock();
	}

	void Interpolate(float dt, XMFLOAT3& targetPos, XMFLOAT4& targetRot)
	{
		mEntryMapMut.lock();
		if (mEntries.empty())
		{
			mEntryMapMut.unlock();
			return;
		}
		mEntryMapMut.unlock();

		if (mPrevEntry.empty)
		{
			EraseOldEntries();

			if (mPrevEntry.empty)
			{
				mEntryMapMut.lock();
				mPrevTimePoint = mEntries.begin()->first;
				mPrevEntry = mEntries.begin()->second;
				mEntryMapMut.unlock();
			}		
			targetPos = mPrevEntry.position;
			targetRot = mPrevEntry.rotation;
			return;
		}

		mEntryMapMut.lock();
		uint64_t nextTp = mEntries.begin()->first;
		Entry next = mEntries.begin()->second;
		mCurrentTime = (uint64_t)((mCurrentTime * PCT) + (mEntries.rbegin()->first * (1.0f - PCT)));
		mEntryMapMut.unlock();

		uint64_t timeBetween = nextTp - mPrevTimePoint;
		float progress = (float)(mCurrentTime - mPrevTimePoint) / timeBetween;

		Print("CurrentTime: ", mCurrentTime);
		Print("timeBetween: ", timeBetween);
		Print("Progress: ", progress);

		targetPos = Vector3::Lerp(mPrevEntry.position, next.position, progress);
		targetRot = Vector4::Slerp(mPrevEntry.rotation, next.rotation, progress);

		EraseOldEntries();
	}

	void Clear()
	{
		mEntryMapMut.lock();
		mEntries.clear();
		mEntryMapMut.unlock();

		mCurrentTime = 0;
		mPrevEntry = Entry{};
	}

private:
	static float GetDurationSec(uint64_t a, uint64_t b)
	{
		return (float)(a - b) / 1'000'000'000.f;
	}

	void EraseOldEntries()
	{
		mEntryMapMut.lock();
		while (mEntries.empty() == false && mCurrentTime > mEntries.begin()->first)
		{
			mPrevTimePoint = mEntries.begin()->first;
			mPrevEntry = mEntries.begin()->second;
			mEntries.erase(mEntries.begin());
		}

		mEntryMapMut.unlock();
	}

private:
	std::map<uint64_t, Entry> mEntries;
	std::mutex mEntryMapMut;

	uint64_t mCurrentTime;
	uint64_t mPrevTimePoint;
	Entry mPrevEntry;

	const float PCT = 0.5f;
};