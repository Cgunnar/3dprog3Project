#pragma once

#include <chrono>

enum class Duration
{
	SECONDS,
	MILLISECONDS,
	MICROSECONDS,
	NANOSECONDS,
};

class Timer
{
public:
	Timer(Duration duration = Duration::MICROSECONDS);
	~Timer() = default;


	

	void start();
	uint64_t stop();

	std::string durationToString() const;
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> m_start, m_stop;
	Duration m_duration;
};

