#pragma once

#include <cstdint>


class Reset
{
public:
	static Reset* getInstance() { return m_instance; }
	static void init(const unsigned long& delay_ms, uint8_t pin, bool inverted = false);

	void update();

private:
	static Reset* m_instance;

	Reset(const unsigned long& delay_ms, uint8_t pin, bool inverted);
	~Reset() = default;

	const uint8_t m_pin;
	const bool m_inverted;
	const unsigned long m_delay_ms;

	volatile bool m_pinState;
	volatile unsigned long m_timeLastChange;

	bool m_armed;
	bool m_resetTriggered = false;

	static void handlePinChange();
	void updatePinState();

	void reboot();
};
