#pragma once

#include <chrono>

class FrameTimer
{
private:
	class InternalTimer
	{
		friend FrameTimer;
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_start, m_end, m_timeZero;
		std::chrono::microseconds m_duration;

		static InternalTimer s_timer;
		static uint64_t m_frameCount;
		

	private:
		InternalTimer();
		~InternalTimer() = default;

		/*InternalTimer(const InternalTimer&) = delete;
		InternalTimer& operator=(const InternalTimer&) = delete;*/

		void start();
		void stop();
	};



public:
	FrameTimer() = default;
	~FrameTimer() = default;
	static void Init();

	enum class Duration
	{
		SECONDS,
		MILLISECONDS,
		MICROSECONDS
	};


	static double NewFrame();

	// Print duration in milliseconds and seconds
	static void Print(std::wstring headerMsg);

	// Get duration: MILLISECONDS, SECONDS
	static double GetDuration(Duration duration);
	static double TimeFromLaunch(Duration duration = Duration::SECONDS);

	static double Dt();
	static uint64_t Frame();
};

