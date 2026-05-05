#include "system.hpp"

#include <Arduino.h>

#ifdef ESP32
#include <esp_heap_caps.h>
#include <esp_system.h>
#endif


System::System(const std::chrono::milliseconds& timeoutWatchdog):
	m_timeoutWatchdog(timeoutWatchdog)
{

}

System::~System()
{

}

void System::init()
{
	const auto timeNow = std::chrono::steady_clock::now();
	m_timeStartup = timeNow;
	m_timeLastUsageUpdate = timeNow;
}

void System::update()
{
	updateMcuUsage();
}

size_t System::getRamHeapSizeTotal()
{
	size_t result = 0;

#ifdef ESP32
	result = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
#endif

	return result;
}

size_t System::getRamHeapSizeFree()
{
	size_t result = 0;

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

std::chrono::nanoseconds System::getUptime()
{
	const auto timeNow = std::chrono::steady_clock::now();
	return (timeNow - m_timeStartup);
}

void System::updateMcuUsage()
{
	const auto timeNow = std::chrono::steady_clock::now();
	const auto timeDiff = std::chrono::duration<float>(timeNow - m_timeLastUsageUpdate);
	m_timeLastUsageUpdate = timeNow;

	// MCU Usage calculated from last loop time and Watchdog timeout
	// -> 100% means that the loop() takes as long as the Watchdog timeout, which would cause a reset
	const float mcuUsage = (timeDiff / m_timeoutWatchdog * 100.0f);

	const float a1 = 1.0f - expf(-timeDiff / std::chrono::minutes(1));
	const float a5 = 1.0f - expf(-timeDiff / std::chrono::minutes(5));
	const float a15 = 1.0f - expf(-timeDiff / std::chrono::minutes(15));

	m_mcuUsage_1min += a1 * (mcuUsage - m_mcuUsage_1min);
	m_mcuUsage_5min += a5 * (mcuUsage - m_mcuUsage_5min);
	m_mcuUsage_15min += a15 * (mcuUsage - m_mcuUsage_15min);
}
