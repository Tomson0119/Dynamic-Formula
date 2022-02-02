#pragma once

#include <chrono>

class Timer
{
private:
	typedef std::chrono::high_resolution_clock clock;

public:
	Timer();
	~Timer();
	
	void Start();
	void Tick();

	float GetElapsedTime() const;
	float GetTotalTime() const;	

private:
	clock::time_point m_start;
	clock::time_point m_prev;
	clock::time_point m_curr;

	float m_elapsed;
};