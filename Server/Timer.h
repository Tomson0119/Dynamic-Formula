#pragma once


class Timer
{
public:
	Timer();
	~Timer();

	void Start();
	void Tick();

	float GetElapsed() const;

private:
	Clock::time_point mPrev;
	Clock::time_point mCurr;

	std::chrono::milliseconds mElapsed;
};


enum class EVENT_TYPE : char
{
	PHYSICS = 0,
	BROADCAST
};

class TimerQueue
{
public:
	struct TimerEvent
	{
		Clock::time_point StartTime;
		EVENT_TYPE Type;
		int WorldID;

		TimerEvent();
		TimerEvent(std::chrono::milliseconds duration, EVENT_TYPE type, int worldID);

		constexpr bool operator<(const TimerEvent& other) const;
	};

public:
	TimerQueue();
	~TimerQueue();

	void Start(class InGameServer* ptr);
	void AddTimerEvent(const TimerEvent& evnt);
	static void TimerThreadFunc(TimerQueue& timer);

private:
	std::atomic_bool mLoop;
	std::thread mThread;

	concurrency::concurrent_priority_queue<TimerEvent> mQueue;

	class InGameServer* mGameServerPtr;
};