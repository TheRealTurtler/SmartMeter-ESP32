#pragma once

#include <Arduino.h>
#include <functional>


class SMLReader
{
public:
	SMLReader(HardwareSerial& serial, uint8_t pinRx, uint8_t pinTx);

	void init();
	void update();

private:
	HardwareSerial& m_serial;

	const uint8_t m_pinRx;
	const uint8_t m_pinTx;

	struct ObisHandler
	{
		unsigned char obis[6];
		std::function<void()> handler;
	};

	static const std::vector<ObisHandler> m_vecObisHandlers;

	bool readByte(unsigned char byte);
};
