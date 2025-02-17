#include "timer.hpp"
#include "Arduino.h"


Timer::Timer()
{
}

Timer::~Timer()
{
	stop();
}

void Timer::setCallback(callback_fc callback)
{
	m_callback = callback;
}

void Timer::setInterval_us(unsigned long interval)
{
	m_interval = interval;
}

void Timer::setInterval_ms(unsigned long interval)
{
	setInterval_us(interval * 1000);
}

void Timer::setInterval_s(unsigned long interval)
{
	setInterval_ms(interval * 1000);
}

void Timer::start()
{
	m_timeLast = micros();
	m_started = true;
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
		m_timeLast = time_us;
		m_callback();
	}
}
