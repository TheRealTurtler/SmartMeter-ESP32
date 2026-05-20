#include "watchdog.hpp"
#include <Arduino.h>

#ifdef ESP32
#include <esp_task_wdt.h>
#endif

Watchdog::Watchdog(const std::chrono::milliseconds& timeout):
	m_timeout(timeout)
{

}

void Watchdog::init()
{
#ifdef ESP32
	esp_task_wdt_config_t wdtConfig;
	wdtConfig.timeout_ms = m_timeout.count();
	wdtConfig.idle_core_mask = 0;
	wdtConfig.trigger_panic = true;
	esp_task_wdt_reconfigure(&wdtConfig);

	// Watchdog is automatically reset whenever loop() is called
	// -> No manual resets are necessary
	// -> This is also the reason, why _reconfigure() instead of _init() has to be called
	//    -> Watchdog is already initialized with a default timeout of 5s
	enableLoopWDT();
#endif
}

void Watchdog::update()
{
#ifdef ESP32

#endif
}
