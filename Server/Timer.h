#pragma once

enum class EVENT_TYPE : char
{
	PHYSICS = 0,
	BROADCAST
};

class InGameServer;

class Timer
{
public:
	using Clock = std::chrono::high_resolution_clock;

	struct TimerEvent
	{
		Clock::time_point StartTime;
		EVENT_TYPE Type;
		int WorldID;
		float TimeStep;

		TimerEvent();
		TimerEvent(std::chrono::milliseconds duration, 
			EVENT_TYPE type, int worldID, float timeStep);

		constexpr bool operator<(const TimerEvent& other) const;
	};

public:
	Timer();
	~Timer();

	void Start(InGameServer* ptr);

	void AddTimerEvent(const TimerEvent& evnt);
	static void TimerThreadFunc(Timer& timer);

private:
	Clock::time_point m_start;
	Clock::time_point m_prev;
	Clock::time_point m_curr;

	float m_elapsed;

	std::atomic_bool mLoop;
	std::thread m_thread;

	concurrency::concurrent_priority_queue<TimerEvent> mTimerQueue;

	InGameServer* mGameServerPtr;
};