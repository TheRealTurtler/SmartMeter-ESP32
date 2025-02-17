#include "heartbeat.hpp"
#include <Arduino.h>


Heartbeat::Heartbeat(unsigned long interval_ms, uint8_t pin, bool inverted):
	m_pin(pin),
	m_inverted(inverted)
{
	auto cb = [this]() { this->callbackHeartbeat(); };

	m_timer.setCallback(cb);
	m_timer.setInterval_ms(interval_ms);
}

void Heartbeat::init()
{
	// Set pin to output mode
	pinMode(m_pin, OUTPUT);

	// Disable output
	digitalWrite(m_pin, m_inverted ? HIGH : LOW);
}

void Heartbeat::start()
{
	// Start sequence
	// -> This way it is easy to tell when the program restarts (e.g. due to watchdog)
	pattern(STARTUP, 5);

	m_timer.start();
}

void Heartbeat::stop()
{
	m_timer.stop();
}

void Heartbeat::update()
{
	m_timer.update();
}

void Heartbeat::pattern(PATTERN pat, uint8_t repeat)
{
	switch (pat)
	{
	case STARTUP:
		patternStartup(repeat);
		break;
	case FAST_1:
		patternFast1(repeat);
		break;
	case SLOW_1:
		patternSlow1(repeat);
		break;

	default:
		break;
	}
}

void Heartbeat::togglePin()
{
	m_lastState = !m_lastState;
	digitalWrite(m_pin, m_inverted ? !m_lastState : m_lastState);
}

void Heartbeat::callbackHeartbeat()
{
	if (m_pause)
		return;

	togglePin();
}

void Heartbeat::patternStartup(uint8_t repeat)
{
	if (m_lastState)
	{
		togglePin();
		delay(50);
	}

	for (uint8_t ctr = 0; ctr < repeat; ++ctr)
	{
		togglePin();
		delay(50);

		togglePin();
		delay(50);
	}
}

void Heartbeat::patternFast1(uint8_t repeat)
{
	if (m_lastState)
	{
		togglePin();
		delay(50);
	}

	for (uint8_t ctr = 0; ctr < repeat; ++ctr)
	{
		togglePin();
		delay(50);

		togglePin();
		delay(200);
	}
}

void Heartbeat::patternSlow1(uint8_t repeat)
{
	if (m_lastState)
	{
		togglePin();
		delay(50);
	}

	for (uint8_t ctr = 0; ctr < repeat; ++ctr)
	{
		togglePin();
		delay(500);

		togglePin();
		delay(1000);
	}
}
