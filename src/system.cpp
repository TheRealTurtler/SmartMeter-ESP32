#include "system.hpp"

#include <Arduino.h>

#ifdef ESP32
#include <esp_heap_caps.h>
#include <esp_system.h>
#endif


System::System(const uint32_t& timeoutWatchdog):
	m_timeoutWatchdog(timeoutWatchdog)
{

}

System::~System()
{

}

void System::init()
{
	m_timeLast = micros();
}

void System::update()
{
	updateMcuUsage();
}

size_t System::getRamHeapSizeTotal()
{
	uint32_t result = 0;

#ifdef ESP32
	result = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);

#endif

	return result;
}

size_t System::getRamHeapSizeFree()
{
	uint32_t result = 0;

#ifdef ESP32
	result = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
#endif

	return result;
}

size_t System::getRamHeapSizeUsed()
{
	size_t result = 0;

	const size_t sizeTotal = getRamHeapSizeTotal();
	const size_t sizeFree = getRamHeapSizeFree();

	if (sizeTotal >= sizeFree)
		result = (sizeTotal - sizeFree);

	return result;
}

float System::getRamHeapSizePercent()
{
	float result = -1.0f;

	const float sizeTotal = getRamHeapSizeTotal();

	if (sizeTotal > 0)
		result = (getRamHeapSizeUsed() / sizeTotal * 100.0f);

	return result;
}

float System::getTemperature()
{
	return temperatureRead();
}

void System::updateMcuUsage()
{
	const unsigned long timeNow = micros();
	const unsigned long timeDiff = (timeNow - m_timeLast);
	m_timeLast = timeNow;

	// MCU Usage calculated from last loop time and Watchdog timeout
	// -> 100% means that the loop() takes as long as the Watchdog timeout, which would cause a reset
	const float mcuUsage = (100.0f * timeDiff) / (m_timeoutWatchdog * 1000.0f);

	const float dt = (timeDiff / 1000.0f / 1000.0f);

	const float tau1 = 60.0f;		// 1 min
	const float tau5 = 300.0f;		// 5 min
	const float tau15 = 900.0f;		// 15 min

	const float a1 = 1.0f - expf(-dt / tau1);
	const float a5 = 1.0f - expf(-dt / tau5);
	const float a15 = 1.0f - expf(-dt / tau15);

	m_mcuUsage_1min += a1 * (mcuUsage - m_mcuUsage_1min);
	m_mcuUsage_5min += a5 * (mcuUsage - m_mcuUsage_5min);
	m_mcuUsage_15min += a15 * (mcuUsage - m_mcuUsage_15min);
}
