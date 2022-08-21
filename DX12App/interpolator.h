#include "stdafx.h"

class Interpolator
{
public:
	struct Entry
	{
		bool empty;
		uint64_t timepointS;
		uint64_t timepointC;
		XMFLOAT3 position;
		XMFLOAT4 rotation;

		Entry()
			: empty{ true },
			timepointS{ 0 },
			timepointC{ 0 },
			position{},
			rotation{}
		{
		}

		Entry(uint64_t tp_s, uint64_t tp_c, const XMFLOAT3& pos, const XMFLOAT4& rot)
			: empty{ false }
		{
			timepointS = tp_s;
			timepointC = tp_c;
			position = pos;
			rotation = rot;
		}

		void Set(uint64_t tp_s, uint64_t tp_c, const XMFLOAT3& pos, const XMFLOAT4& rot)
		{
			empty = false;
			timepointS = tp_s;
			timepointC = tp_c;
			position = pos;
			rotation = rot;
		}
	};

public:
	Interpolator()
	{
	}

	void Enqueue(uint64_t timePointS, const XMFLOAT3& pos, const XMFLOAT4& rot)
	{
		mMut.lock();
		uint64_t time_now = ConvertNsToMs(Clock::now().time_since_epoch().count());

		if (mPrevEntry.empty) {
			mPrevEntry.Set(timePointS, time_now, pos, rot);
		}
		else {
			if (mEntryQue.empty() && mPrevEntry.timepointS < timePointS) {
				mEntryQue.push({ timePointS, time_now, pos, rot });
				Log::Print("queue size: ", mEntryQue.size());
			} 
			else if(mEntryQue.empty()==false && mEntryQue.back().timepointS < timePointS) {
				mEntryQue.push({ timePointS, time_now, pos, rot });
				Log::Print("queue size: ", mEntryQue.size());
			}
			else {
				Log::Print("Wrong order");
			}
		}
		mMut.unlock();
	}

	void Interpolate(float dt, int64_t clockDelta, XMFLOAT3& targetPos, XMFLOAT4& targetRot)
	{
		mMut.lock();
		if (mPrevEntry.empty) {
			mMut.unlock();
			return;
		}

		if (mNextEntry.empty) {
			if (mEntryQue.empty() == false) {
				mNextEntry = mEntryQue.front();
				mEntryQue.pop();
			}
			else {
				targetPos = mPrevEntry.position;
				targetRot = mNextEntry.rotation;
				mMut.unlock();
				return;
			}
		}

		uint64_t time_now = ConvertNsToMs(Clock::now().time_since_epoch().count());
		uint64_t between = mNextEntry.timepointS - mPrevEntry.timepointS;
		uint64_t progress = time_now - mPrevEntry.timepointC;
		if (mPrevEntry.timepointS < mPrevEntry.timepointC) {
			uint64_t diff = mPrevEntry.timepointC - mPrevEntry.timepointS;

			progress -= diff;
		}
		else {
			uint64_t diff = mPrevEntry.timepointS - mPrevEntry.timepointC;
			progress += diff;
		}

		if (between <= 0) {
			if (mEntryQue.empty() == false) {
				mPrevEntry = mNextEntry;
				mNextEntry = mEntryQue.front();
				mEntryQue.pop();
			}
			mMut.unlock();
			return;
		}
		
		if (progress > between) {
			if (mEntryQue.empty() == false) {
				mPrevEntry = mNextEntry;
				mNextEntry = mEntryQue.front();
				mEntryQue.pop();

				between = mNextEntry.timepointS - mPrevEntry.timepointS;

				progress = time_now - mPrevEntry.timepointC;
				if (mPrevEntry.timepointS < mPrevEntry.timepointC) {
					uint64_t diff = mPrevEntry.timepointC - mPrevEntry.timepointS;
					progress -= diff;
				}
				else {
					uint64_t diff = mPrevEntry.timepointS - mPrevEntry.timepointC;
					progress += diff;
				}

				Log::Print("between: ", between);
				Log::Print("progress: ", progress);
			}
		}

		float ratio = (float)progress / between;
		Log::Print("ratio: ", ratio);

		targetPos = Vector3::Lerp(mPrevEntry.position, mNextEntry.position, ratio);
		targetRot = Vector4::Slerp(mPrevEntry.rotation, mNextEntry.rotation, ratio);
		
		mMut.unlock();
	}

	void Clear()
	{
		mMut.lock();

		mPrevEntry.empty = true;
		mNextEntry.empty = true;
		
		std::queue<Entry> temp{};
		std::swap(mEntryQue, temp);
		
		mMut.unlock();
	}

private:
	std::queue<Entry> mEntryQue;

	Entry mPrevEntry;
	Entry mNextEntry;

	std::mutex mMut;
};