#include "timer.hpp"
#include "Arduino.h"


Timer::Timer()
{ }

Timer::~Timer()
{
	stop();
}

void Timer::setCallback(callback_fc callback)
{
	m_callback = callback;
}

void Timer::setSingleShot(bool singleshot)
{
	m_singleshot = singleshot;
}

void Timer::setInterval_us(const unsigned long& interval)
{
	m_interval = interval;
}

void Timer::setInterval_ms(const unsigned long& interval)
{
	setInterval_us(interval * 1000UL);
}

void Timer::setInterval_s(const unsigned long& interval)
{
	setInterval_us(interval * 1000UL * 1000UL);
}

void Timer::start()
{
	m_timeLast = micros();
	m_started = true;
}

void Timer::start_us(const unsigned long& interval)
{
	setInterval_us(interval);
	start();
}

void Timer::start_ms(const unsigned long& interval)
{
	setInterval_ms(interval);
	start();
}

void Timer::start_s(const unsigned long& interval)
{
	setInterval_s(interval);
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

	const unsigned long time_us = micros();

	unsigned long timeDiff = 0;

	if (m_timeLast > time_us)
		timeDiff = (ULONG_MAX - time_us + time_us);
	else
		timeDiff = (time_us - m_timeLast);

	if (timeDiff > m_interval)
	{
		if (m_singleshot)
			stop();

		m_timeLast = time_us;
		m_callback();
	}
}
