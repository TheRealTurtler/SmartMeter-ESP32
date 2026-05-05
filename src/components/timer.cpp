#include "timer.hpp"
#include "Arduino.h"


Timer::Timer()
{ }

Timer::~Timer()
{
	stop();
}

void Timer::addCallback(const callback_fc& callback)
{
	m_vecFuncCallbacks.push_back(callback);
}

void Timer::start()
{
	m_timeLast = std::chrono::steady_clock::now();
	m_started = true;
}

void Timer::start(const std::chrono::nanoseconds& interval)
{
	setInterval(interval);
	start();
}

void Timer::stop()
{
	m_started = false;
}

void Timer::update()
{
	if (!m_started)
		return;

	const auto timeNow = std::chrono::steady_clock::now();
	const auto timeDiff = (timeNow - m_timeLast);

	if (timeDiff > m_interval)
	{
		if (m_singleshot)
			stop();

		m_timeLast = timeNow;

		for (const auto& func : m_vecFuncCallbacks)
		{
			func();
		}
	}
}
