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
	mPrev = Clock::now();
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
	mQueue.push(evnt);
}

void TimerQueue::TimerThreadFunc(TimerQueue& timer)
{
	TimerEvent ev{};
	while (timer.mLoop)
	{
		while (timer.mQueue.empty() == false)
		{
			if (timer.mQueue.try_pop(ev) == false)
				continue;

			if (ev.Type == EVENT_TYPE::PHYSICS)
			{
				auto now = Clock::now();			
				std::this_thread::sleep_for(ev.StartTime - now);
				timer.mGameServerPtr->PostPhysicsOperation(ev.WorldID);
			}
			/*else if (ev.Type == EVENT_TYPE::BROADCAST)
			{
				auto now = Clock::now();
				if (now < ev.StartTime)
				{
					timer.mQueue.push(ev);
					continue;
				}
				timer.mGameServerPtr->BroadcastTransforms(ev.WorldID);
			}*/
		}
		std::this_thread::sleep_for(10ms);
	}
}