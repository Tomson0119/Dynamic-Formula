#include "common.h"
#include "Timer.h"
#include "InGameServer.h"

//
//	Timer
//
Timer::Timer()
	: mPrev{}, mCurr{}, mElapsed{}
{
}

Timer::~Timer()
{
}

void Timer::Start()
{
	mPrev = Clock::now();
	mCurr = Clock::now();
}

void Timer::Tick()
{
	mCurr = Clock::now();
	mElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(mCurr - mPrev);
	mPrev = mCurr;
}

float Timer::GetElapsed() const
{
	return (float)mElapsed.count() / 1000.0f;
}


//
//	TimerEvent
//
TimerQueue::TimerEvent::TimerEvent()
	: Type{}, WorldID{ -1 } 
{
}

TimerQueue::TimerEvent::TimerEvent(std::chrono::milliseconds duration, EVENT_TYPE type, int worldID)
	: Type{ type }, WorldID{ worldID }
{
	StartTime = Clock::now() + duration;
}

constexpr bool TimerQueue::TimerEvent::operator<(const TimerEvent& other) const
{
	return (StartTime > other.StartTime);
}


//
//  TimerQueue
//
TimerQueue::TimerQueue()
	: mLoop{ true }, mGameServerPtr{ nullptr }
{
}

TimerQueue::~TimerQueue()
{
	mLoop = false;
	if (mThread.joinable()) mThread.join();
}

void TimerQueue::Start(InGameServer* ptr)
{
	mGameServerPtr = ptr;
	mThread = std::thread{ TimerThreadFunc, std::ref(*this) };
}

void TimerQueue::AddTimerEvent(const TimerEvent& evnt)
{
	std::unique_lock<std::mutex> lock{ mQueueMut };
	mQueue.push(evnt);
}

bool TimerQueue::IsQueueEmpty()
{
	std::unique_lock<std::mutex> lock{ mQueueMut };
	return mQueue.empty();
}

void TimerQueue::TimerThreadFunc(TimerQueue& timer)
{
	TimerEvent ev{};
	while (timer.mLoop)
	{
		while (timer.IsQueueEmpty()==false)
		{
			{
				std::unique_lock<std::mutex> lock{ timer.mQueueMut };
				ev = timer.mQueue.front();

				auto now = Clock::now();
				if (ev.StartTime <= now)
				{
					timer.mQueue.pop();
				}
				else continue;
			}

			if (ev.Type == EVENT_TYPE::PHYSICS)
			{
				timer.mGameServerPtr->PostPhysicsOperation(ev.WorldID);
			}
		}
	}
}