#pragma once

#include <Arduino.h>
#include <functional>
#include <sml.h>
#include "datacollector.hpp"


class SMLReader
{
public:
	SMLReader(DataCollector* const dc, HardwareSerial& serial, uint8_t pinRx, uint8_t pinTx);

	void init();
	void update();

private:
	DataCollector* const m_dc;
	HardwareSerial& m_serial;

	const uint8_t m_pinRx;
	const uint8_t m_pinTx;

	struct ObisData
	{
		const unsigned char obis[6];
		const DATA_POINT_SMARTMETER dp;
		const sml_units_t unit;
	};

	static const std::vector<ObisData> m_vecObisData;

	std::map<DATA_POINT_SMARTMETER, double> m_mapValues;
	int64_t m_tsLastUpdate = -1;

	bool readByte(unsigned char byte);
	void obisHandler(const ObisData& obisValue);
};
