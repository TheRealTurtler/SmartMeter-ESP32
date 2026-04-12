#pragma once

#include <stdint.h>

class Watchdog
{
public:
	Watchdog(uint32_t timeoutMs);
	~Watchdog() = default;

	void init();
	void update();

	uint32_t& getTimeout() { return m_timeoutMs; }

private:
	uint32_t m_timeoutMs;
};
