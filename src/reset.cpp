#include "reset.hpp"

#include <Arduino.h>
#include "config/config.hpp"


Reset* Reset::m_instance = nullptr;


Reset::Reset(const unsigned long& delay_ms, uint8_t pin, bool inverted):
	m_pin(pin),
	m_inverted(inverted),
	m_delay_ms(delay_ms)
{
	// Set pin to input mode
	// -> No pullup / pulldown necessary because an external one is used
	pinMode(m_pin, INPUT);

	updatePinState();

	// Only arm reset button when state is LOW
	m_armed = (!m_pinState);
}

void Reset::init(const unsigned long& delay_ms, uint8_t pin, bool inverted)
{
	if (!m_instance)
	{
		m_instance = new Reset(delay_ms, pin, inverted);
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
			const unsigned long timeDiff = (millis() - m_timeLastChange);

			if (timeDiff > m_delay_ms)
			{
				Serial.println("!!!!! RESET TRIGGERED !!!!!");

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
	m_timeLastChange = millis();
}

void Reset::reboot()
{
#ifdef ESP32
	ESP.restart();
#endif

	// If no reboot option is available, block execution until watchdog triggeres
	while (true) { }
}
