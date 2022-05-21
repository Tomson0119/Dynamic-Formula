#include "stdafx.h"
#include <queue>

class Interpolator
{
private:
	using TimeStamp = float;
	const float DELAY = 0.07f;

public:
	struct Entry
	{
		TimeStamp timeStamp;

		XMFLOAT3 position;
		XMFLOAT4 rotation;

		Entry(const XMFLOAT3& pos, const XMFLOAT4& rot)
		{
			timeStamp = GetTimeStampNow();
			position = pos;
			rotation = rot;
		}
	};

public:
	Interpolator()
		: mPieceStartTime{ -1 },
		  mCurrentTime{ -1 },
		  mInterpStartTime{ 0 },
		  mInterpStartPos{ },
		  mInterpStartRot{ }
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
		OutputDebugStringA(("CurrentTime: " + std::to_string(mCurrentTime) + "\n").c_str());
		if (ShouldDelay())
		{
			mCurrentTime = -1.0f;
			return;
		}

		if (mCurrentTime >= 0.0f)
		{
			mCurrentTime += dt;
		}

		mEntryQueueMut.lock();
		if (mEntryQueue.empty())
		{
			mEntryQueueMut.unlock();
			return;
		}
		
		Entry prevEntry = mEntryQueue.front();
		mEntryQueueMut.unlock();

		if (mCurrentTime < 0.0f)
		{
			mCurrentTime = std::max(prevEntry.timeStamp - DELAY, 0.0f);
			mInterpStartTime = mCurrentTime;
			mInterpStartPos = targetPos;
			mInterpStartRot = targetRot;
		}

		if (mCurrentTime < prevEntry.timeStamp)
		{
			float timeBetween = prevEntry.timeStamp - mInterpStartTime;
			float timeSincePrev = mCurrentTime - mInterpStartTime;

			float delta = timeSincePrev / timeBetween;
			targetPos = Vector3::Lerp(mInterpStartPos, prevEntry.position, delta);
			targetRot = Vector4::Slerp(mInterpStartRot, prevEntry.rotation, delta);
			return;
		}

		Entry nextEntry = prevEntry;
		mEntryQueueMut.lock();
		while (mCurrentTime > nextEntry.timeStamp && mEntryQueue.empty() == false)
		{
			Entry oldest = mEntryQueue.front();
			mEntryQueue.pop();

			if (mEntryQueue.empty())
			{
				nextEntry = oldest;
				prevEntry = nextEntry;
			}
			else
			{
				prevEntry = oldest;
				nextEntry = mEntryQueue.front();
			}
		}
		mEntryQueueMut.unlock();

		float timeBetween = nextEntry.timeStamp - prevEntry.timeStamp;
		float timeSincePrev = mCurrentTime - prevEntry.timeStamp;

		if (timeBetween == 0.0f)
		{
			timeBetween = 1.0f;
			timeSincePrev = 1.0f;
		}

		float delta = timeSincePrev / timeBetween;
		targetPos = Vector3::Lerp(prevEntry.position, nextEntry.position, delta);
		targetRot = Vector4::Slerp(prevEntry.rotation, nextEntry.rotation, delta);

		mInterpStartTime = mCurrentTime;
		mInterpStartPos = prevEntry.position;
		mInterpStartRot = prevEntry.rotation;
	}

private:
	bool ShouldDelay()
	{
		if (IsEmpty())
		{
			mPieceStartTime = -1;
			return true;
		}

		if (mPieceStartTime < 0)
		{
			mEntryQueueMut.lock();
			mPieceStartTime = mEntryQueue.front().timeStamp;
			mEntryQueueMut.unlock();
		}

		auto now = Clock::now().time_since_epoch().count();
		return (now - mPieceStartTime) < DELAY;
	}

	bool IsEmpty()
	{
		mEntryQueueMut.lock();
		bool res = mEntryQueue.empty();
		mEntryQueueMut.unlock();
		return res;
	}

	static TimeStamp GetTimeStampNow()
	{
		auto nowMsec = std::chrono::duration_cast<
			std::chrono::milliseconds>(Clock::now().time_since_epoch()).count();

		TimeStamp nowSec = (TimeStamp)nowMsec / 1000.0f;
		return nowSec;
	}

private:
	std::queue<Entry> mEntryQueue;
	std::mutex mEntryQueueMut;

	TimeStamp mPieceStartTime;
	TimeStamp mCurrentTime;

	TimeStamp mInterpStartTime;
	XMFLOAT3 mInterpStartPos;
	XMFLOAT4 mInterpStartRot;
};