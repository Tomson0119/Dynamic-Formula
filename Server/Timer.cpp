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
	TimerEvent ev{};
	TimerEvent nextEv{};
	while (timer.mLoop)
	{
		while (timer.mTimerQueue.empty() == false)
		{
			if (timer.mTimerQueue.try_pop(ev) == false)
				continue;

			if (ev.Type == EVENT_TYPE::PHYSICS)
			{
				auto now = Clock::now();			
				std::this_thread::sleep_for(ev.StartTime - now);
				std::cout << "physics event!\n";
				timer.mGameServerPtr->PostIOCPOperation(ev.WorldID, OP::PHYSICS, ev.TimeStep);
			}
			else if (ev.Type == EVENT_TYPE::BROADCAST)
			{
				auto now = Clock::now();
				if (now < ev.StartTime)
				{
					timer.mTimerQueue.push(ev);
					continue;
				}
				std::cout << "broadcast event!\n";
				timer.mGameServerPtr->PostIOCPOperation(ev.WorldID, OP::BROADCAST);
			}
		}
		std::this_thread::sleep_for(10ms);
	}
}