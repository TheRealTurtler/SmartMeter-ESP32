#pragma once

#include "timer.hpp"

class Heartbeat
{
public:
	explicit Heartbeat(unsigned long interval_ms, uint8_t pin, bool inverted = false);
	~Heartbeat() = default;

	enum PATTERN
	{
		STARTUP,
		FAST_1,
		SLOW_1,
	};

	void init();

	void start();
	void stop();

	void update();

	void pattern(PATTERN pat, uint8_t repeat);

private:
	const uint8_t m_pin;
	const bool m_inverted;

	Timer m_timer;

	bool m_lastState = false;
	bool m_pause = false;

	void togglePin();

	void callbackHeartbeat();

	void patternStartup(uint8_t repeat);
	void patternFast1(uint8_t repeat);
	void patternSlow1(uint8_t repeat);
};
