#include "pch.h"
#include "FrameTimer.h"


using namespace std;
using namespace chrono;

FrameTimer::InternalTimer FrameTimer::InternalTimer::s_timer;
uint64_t FrameTimer::InternalTimer::m_frameCount = 0;

double FrameTimer::NewFrame()
{
    InternalTimer::s_timer.stop();
    InternalTimer::s_timer.start();
    return Dt();
}

void FrameTimer::Print(std::wstring headerMsg)
{
    std::wstring outputStr(
        L"\n--- " + headerMsg + L" ---\n" +
        L"Elapsed: " + std::to_wstring(GetDuration(Duration::MILLISECONDS)) + L"ms " +
        L"(" + std::to_wstring(GetDuration(Duration::SECONDS)) + L"s).\n"
    );
    std::wcout << outputStr.c_str();
}

double FrameTimer::GetDuration(Duration duration)
{
    switch (duration)
    {
    case Duration::MILLISECONDS:
        return static_cast<double>(InternalTimer::s_timer.m_duration.count() * 1E-03);    // Milliseconds with decimal
        break;
    case Duration::SECONDS:
        return static_cast<double>(InternalTimer::s_timer.m_duration.count() * 1E-06);    // Seconds with decimal
        break;
    case Duration::MICROSECONDS:
        return static_cast<double>(InternalTimer::s_timer.m_duration.count());
        break;

        // Microseconds default
    default:
        return static_cast<double>(InternalTimer::s_timer.m_duration.count());
    }
}

double FrameTimer::TimeFromLaunch(Duration duration)
{
    microseconds micro = duration_cast<microseconds>(InternalTimer::s_timer.m_end - InternalTimer::s_timer.m_timeZero);
    switch (duration)
    {
    case Duration::MILLISECONDS:
        return static_cast<double>(micro.count() * 1E-03);
    case Duration::SECONDS:
        return static_cast<double>(micro.count() * 1E-06);
    case Duration::MICROSECONDS:
        return static_cast<double>(micro.count());
    default:
        return static_cast<double>(InternalTimer::s_timer.m_duration.count());
    }
}

double FrameTimer::Dt()
{
    return InternalTimer::s_timer.m_duration.count() * 1E-06;    // Seconds with decimal
}

uint64_t FrameTimer::Frame()
{
    return InternalTimer::m_frameCount;
}

void FrameTimer::Init()
{
    InternalTimer::s_timer = InternalTimer();
}

FrameTimer::InternalTimer::InternalTimer() :
    m_start(high_resolution_clock::now()),
        m_end(high_resolution_clock::now()),
        m_duration(0), m_timeZero(high_resolution_clock::now())
{       
}

void FrameTimer::InternalTimer::start()
{
    m_start = high_resolution_clock::now();
}

void FrameTimer::InternalTimer::stop()
{
    m_end = high_resolution_clock::now();
    m_duration = duration_cast<microseconds>(m_end - m_start);
    m_frameCount++;
}
