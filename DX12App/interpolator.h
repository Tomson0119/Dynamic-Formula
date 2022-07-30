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

	void Interpolate(float dt, int64_t clockDelta, XMFLOAT3& targetPos, XMFLOAT4& targetRot)
	{
		mEntryMapMut.lock();
		if (mEntries.empty())
		{
			mEntryMapMut.unlock();
			return;
		}
		mEntryMapMut.unlock();

		/*if (mPrevEntry.empty)
		{
			FindNextEntry();

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
		}*/

		auto now = Clock::now().time_since_epoch().count();
		auto now_ms = ConvertNsToMs(now) + clockDelta;

		//mCurrentTime = (uint64_t)(((float)mCurrentTime * PCT) + (float)now_ms * (1.0f - PCT));
		mCurrentTime = now_ms;

		auto next = FindNextEntry();
		if (next.has_value() == false) {
			return;
		}

		uint64_t nextTp = next.value().first;
		Entry nextEntry = next.value().second;

		if (nextTp <= mPrevTimePoint) return;

		uint64_t timeBetween = nextTp - mPrevTimePoint;		
		float progress = (float)(mCurrentTime - mPrevTimePoint) / timeBetween;

		Log::Print("prev time: ", mPrevTimePoint);
		Log::Print("current time: ", mCurrentTime);
		Log::Print("next time: ", nextTp);
		Log::Print("progress: ", progress);

		targetPos = Vector3::Lerp(mPrevEntry.position, nextEntry.position, progress);
		targetRot = Vector4::Slerp(mPrevEntry.rotation, nextEntry.rotation, progress);

		/*Print("prev position: ", mPrevEntry.position);
		Print("current position: ", targetPos);
		Print("next position: ", nextEntry.position);*/
		//Log::Print("-----------------------------------", 0);
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
	std::optional<std::pair<uint64_t, Entry>> FindNextEntry()
	{
		std::unique_lock lock{ mEntryMapMut };
		/*while (mEntries.empty() == false && 
			(mCurrentTime > mEntries.begin()->first))
		{
			mPrevTimePoint = mEntries.begin()->first;
			mPrevEntry = mEntries.begin()->second;
			mEntries.erase(mEntries.begin());
		}*/
		
		//Log::Print("Entries size: ", mEntries.size());

		if (mEntries.empty())
			return std::nullopt;

		auto ret = *mEntries.begin();
		mEntries.erase(mEntries.begin());
		return ret;
	}

private:
	std::map<uint64_t, Entry> mEntries;
	std::mutex mEntryMapMut;

	uint64_t mCurrentTime;
	uint64_t mPrevTimePoint;
	Entry mPrevEntry;

	//const float PCT = 0.8f;
};