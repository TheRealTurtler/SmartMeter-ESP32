#pragma once

#include <cstddef>
#include <cstdint>


class System
{
public:
	System(const uint32_t& timeoutWatchdog);
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

private:
	const uint32_t m_timeoutWatchdog;

	unsigned long m_timeLast = 0;

	float m_mcuUsage_1min = 0.0f;
	float m_mcuUsage_5min = 0.0f;
	float m_mcuUsage_15min = 0.0f;

	void updateMcuUsage();
};
