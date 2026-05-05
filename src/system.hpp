#pragma once

#include <cstddef>
#include <cstdint>
#include <chrono>


class System
{
public:
	System(const std::chrono::milliseconds& timeoutWatchdog);
	~System();

	void init();
	void update();

	static size_t getRamHeapSizeTotal();
	static size_t getRamHeapSizeFree();
	static size_t getRamHeapSizeUsed();
	static float getRamHeapSizePercent();

	float getTemperature();

	const float& getMcuUsage1min() const { return m_mcuUsage_1min; }
	const float& getMcuUsage5min() const { return m_mcuUsage_5min; }
	const float& getMcuUsage15min() const { return m_mcuUsage_15min; }

	std::chrono::nanoseconds getUptime();

private:
	std::chrono::milliseconds m_timeoutWatchdog;

	std::chrono::steady_clock::time_point m_timeStartup;
	std::chrono::steady_clock::time_point m_timeLastUsageUpdate;

	float m_mcuUsage_1min = 0.0f;
	float m_mcuUsage_5min = 0.0f;
	float m_mcuUsage_15min = 0.0f;

	void updateMcuUsage();
};
