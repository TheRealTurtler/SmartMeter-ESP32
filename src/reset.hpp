#pragma once

#include <cstdint>
#include <chrono>


class Reset
{
public:
	static Reset* getInstance() { return m_instance; }
	static void init(const std::chrono::milliseconds& delay, uint8_t pin, bool inverted = false);

	void update();

private:
	static Reset* m_instance;

	Reset(const std::chrono::milliseconds& delay, uint8_t pin, bool inverted);
	~Reset() = default;

	const uint8_t m_pin;
	const bool m_inverted;
	const std::chrono::milliseconds m_delay;

	volatile bool m_pinState;
	std::chrono::steady_clock::time_point m_timeLastChange;

	bool m_armed;
	bool m_resetTriggered = false;

	static void handlePinChange();
	void updatePinState();

	void reboot();
};
