#pragma once

#include <stdint.h>
#include <chrono>

class Watchdog
{
public:
	Watchdog(const std::chrono::milliseconds& timeout);
	~Watchdog() = default;

	void init();
	void update();

	const std::chrono::milliseconds& getTimeout() { return m_timeout; }

private:
	std::chrono::milliseconds m_timeout;
};
