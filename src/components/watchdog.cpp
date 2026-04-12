#include "watchdog.hpp"

#ifdef ESP32
#include <esp_task_wdt.h>
#endif

Watchdog::Watchdog(uint32_t timeoutMs):
	m_timeoutMs(timeoutMs)
{

}

void Watchdog::init()
{
#ifdef ESP32
	esp_task_wdt_config_t wdtConfig;
	wdtConfig.timeout_ms = m_timeoutMs;
	wdtConfig.idle_core_mask = 0;
	wdtConfig.trigger_panic = true;
	esp_task_wdt_init(&wdtConfig);
	esp_task_wdt_add(nullptr);
	esp_task_wdt_reset();
#endif
}

void Watchdog::update()
{
#ifdef ESP32
	esp_task_wdt_reset();
#endif
}
