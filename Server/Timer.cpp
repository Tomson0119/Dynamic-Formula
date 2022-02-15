#include "common.h"
#include "Timer.h"
#include "InGameServer.h"

Timer::TimerEvent::TimerEvent()
	: Type{}, WorldID{ -1 }, TimeStep{ 0.0f } 
{
}

Timer::TimerEvent::TimerEvent(
	std::chrono::milliseconds duration, 
	EVENT_TYPE type, int worldID, float timeStep)
	: Type{ type }, WorldID{ worldID }, TimeStep{ timeStep }
{
	StartTime = Clock::now() + duration;
}

constexpr bool Timer::TimerEvent::operator<(const TimerEvent& other) const
{
	return (StartTime > other.StartTime);
}


//
//  Timer 
//
Timer::Timer()
	: m_start{}, m_prev{}, m_curr{}, 
	  m_elapsed{}, mLoop{ true }, mGameServerPtr{ nullptr }
{
}

Timer::~Timer()
{
	mLoop = false;
	if (m_thread.joinable()) m_thread.join();
}

void Timer::Start(InGameServer* ptr)
{
	mGameServerPtr = ptr;
	m_thread = std::thread{ TimerThreadFunc, std::ref(*this) };
}

void Timer::AddTimerEvent(const TimerEvent& evnt)
{
	mTimerQueue.push(evnt);
}

void Timer::TimerThreadFunc(Timer& timer)
{
	while (timer.mLoop)
	{
		while (timer.mTimerQueue.empty() == false)
		{
			TimerEvent ev;
			timer.mTimerQueue.try_pop(ev);

			auto now = Clock::now();
			if (now < ev.StartTime)
				std::this_thread::sleep_for(ev.StartTime - Clock::now());
			
			if (ev.Type == EVENT_TYPE::PHYSICS)
			{
				timer.mGameServerPtr->PostPhysicsOperation(ev.WorldID, ev.TimeStep);
			}		
		}
		std::this_thread::sleep_for(10ms);
	}
}