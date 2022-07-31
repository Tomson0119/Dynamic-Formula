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
			: empty{ true }, position{}, rotation{}
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

	void Enqueue(uint64_t timePoint, const XMFLOAT3& pos, const XMFLOAT4& rot)
	{
		mEntryQueueMut.lock();
		mEntryQueue.push(Entry(pos, rot));
		mEntryQueueMut.unlock();
	}

	void Interpolate(float dt, uint64_t timePoint, XMFLOAT3& targetPos, XMFLOAT4& targetRot)
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
		}
	}

	void Clear()
	{
		mEntryQueueMut.lock();
		std::queue<Entry> temp{};
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

//#include "stdafx.h"
//
//class Interpolator
//{
//public:
//	struct Entry
//	{
//		bool empty;
//		XMFLOAT3 position;
//		XMFLOAT4 rotation;
//
//		Entry()
//			: empty{ true },
//			position{},
//			rotation{}
//		{
//		}
//
//		Entry(const XMFLOAT3& pos, const XMFLOAT4& rot)
//			: empty{ false }
//		{
//			position = pos;
//			rotation = rot;
//		}
//	};
//
//public:
//	Interpolator()
//		: mCurrentTime{ 0.0f }, mPrevTimePoint{ 0 }
//	{
//	}
//
//	void Enqueue(uint64_t timePoint, const XMFLOAT3& pos, const XMFLOAT4& rot)
//	{
//		mEntryMapMut.lock();
//		mEntries.insert({ timePoint, Entry(pos, rot) });
//		mEntryMapMut.unlock();
//	}
//
//	void Interpolate(float dt, int64_t clockDelta, XMFLOAT3& targetPos, XMFLOAT4& targetRot)
//	{
//		mEntryMapMut.lock();
//		if (mEntries.empty())
//		{
//			mEntryMapMut.unlock();
//			return;
//		}
//		mEntryMapMut.unlock();
//
//		if (mPrevEntry.empty)
//		{
//			mEntryMapMut.lock();
//			mPrevTimePoint = mEntries.begin()->first;
//			mPrevEntry = mEntries.begin()->second;
//			mEntryMapMut.unlock();
//
//			targetPos = mPrevEntry.position;
//			targetRot = mPrevEntry.rotation;
//			
//			mCurrentTime = 0.0f;
//			return;
//		}
//
//		auto now = Clock::now().time_since_epoch().count();
//		auto now_ms = ConvertNsToMs(now) + clockDelta;
//
//		auto next = FindNextEntry();
//		if (next.has_value()) {
//			mNextTimePoint = next.value().first;
//			mNextEntry = next.value().second;
//		}
//
//		uint64_t timeBetween = mNextTimePoint - mPrevTimePoint;
//
//		//mCurrentTime += dt;
//		float progress = ((float)now_ms) / timeBetween;
//
//		Log::Print("timeBetween: ", timeBetween);
//		Log::Print("now_ms: ", now_ms);
//		Log::Print("progress: ", progress);
//		Log::Print("---------------------------------");
//
//		targetPos = Vector3::Lerp(mPrevEntry.position, mNextEntry.position, progress);
//		targetRot = Vector4::Slerp(mPrevEntry.rotation, mNextEntry.rotation, progress);
//	}
//
//	void Clear()
//	{
//		mEntryMapMut.lock();
//		mEntries.clear();
//		mEntryMapMut.unlock();
//
//		mCurrentTime = 0;
//		mPrevEntry = Entry{};
//	}
//
//private:
//	std::optional<std::pair<uint64_t, Entry>> FindNextEntry()
//	{
//		std::unique_lock lock{ mEntryMapMut };
//		/*while (mEntries.empty() == false &&
//			(mCurrentTime > mEntries.begin()->first))
//		{
//			mPrevTimePoint = mEntries.begin()->first;
//			mPrevEntry = mEntries.begin()->second;
//			mEntries.erase(mEntries.begin());
//		}*/
//
//		if (mEntries.empty())
//			return std::nullopt;
//
//		auto ret = *mEntries.begin();
//		mEntries.erase(mEntries.begin());
//		return ret;
//	}
//
//private:
//	std::map<uint64_t, Entry> mEntries;
//	std::mutex mEntryMapMut;
//
//	uint64_t mPrevTimePoint;
//	uint64_t mNextTimePoint;
//	Entry mPrevEntry;
//	Entry mNextEntry;
//
//	float mCurrentTime;
//	const float PCT = 0.05f;
//};