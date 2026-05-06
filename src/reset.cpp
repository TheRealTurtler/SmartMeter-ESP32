#include "reset.hpp"

#include <Arduino.h>
#include "config/config.hpp"


Reset* Reset::m_instance = nullptr;


Reset::Reset(const std::chrono::milliseconds& delay, uint8_t pin, bool inverted):
	m_pin(pin),
	m_inverted(inverted),
	m_delay(delay)
{
	// Set pin to input mode
	// -> No pullup / pulldown necessary because an external one is used
	pinMode(m_pin, INPUT);

	updatePinState();

	// Only arm reset button when state is LOW
	m_armed = (!m_pinState);

	m_timeLastChange = std::chrono::steady_clock::now();
}

void Reset::init(const std::chrono::milliseconds& delay, uint8_t pin, bool inverted)
{
	if (!m_instance)
	{
		m_instance = new Reset(delay, pin, inverted);
		attachInterrupt(digitalPinToInterrupt(pin), handlePinChange, CHANGE);
	}
}

void Reset::update()
{
	// When a reset is triggered, the button is probably still pressed after restart
	// -> Wait for user to release and press again
	if (!m_armed)
	{
		if (m_pinState)
			return;

		m_armed = true;
	}

	if (!m_resetTriggered)
	{
		static bool last = m_pinState;

		if (m_pinState)
		{
			const auto timeNow = std::chrono::steady_clock::now();
			const auto timeDiff = (timeNow - m_timeLastChange);

			if (timeDiff > m_delay)
			{
				log_w("!!!!! RESET TRIGGERED !!!!!");

				Config::resetConfigAll();
				m_resetTriggered = true;
				reboot();
			}
		}
	}
}

void Reset::handlePinChange()
{
	if (!m_instance)
		return;

	m_instance->updatePinState();
}

void Reset::updatePinState()
{
	const bool rawState = digitalRead(m_pin);

	m_pinState = (m_inverted ? !rawState : rawState);
	m_timeLastChange = std::chrono::steady_clock::now();
}

void Reset::reboot()
{
#ifdef ESP32
	ESP.restart();
#endif

	// If no reboot option is available, block execution until watchdog triggeres
	while (true) { }
}
