#include <Arduino.h>
#include <chrono>
#include "time.h"
#include "components/heartbeat.hpp"
#include "components/watchdog.hpp"
#include "components/timer.hpp"
#include "web/client/httpclient.hpp"
#include "web/server/httpserver.hpp"
#include "smlreader.hpp"
#include "system.hpp"
#include "reset.hpp"
#include "networking.hpp"


constexpr uint8_t PIN_LED_BUILTIN = BUILTIN_LED;		// Pin 8
constexpr uint8_t PIN_LED = 6;
constexpr uint8_t PIN_RX = 20;
constexpr uint8_t PIN_TX = 21;
constexpr uint8_t PIN_BUTTON_RESET = BOOT_PIN;			// Pin 9

Watchdog wd(std::chrono::seconds(10));
Heartbeat hb(std::chrono::seconds(1), PIN_LED, true);

System sys(wd.getTimeout());

DataCollector dc(std::chrono::minutes(5));
SMLReader sml(&dc, Serial1, PIN_RX, PIN_TX);

HttpAPI api;
HttpClient client(api);
HttpServer server(api, dc, 80);


void printResetReason(const esp_reset_reason_t reason)
{
	std::string strReason = "";

	switch (reason)
	{
	case ESP_RST_UNKNOWN:
		strReason = "Unknown";
		break;
	case ESP_RST_POWERON:
		strReason = "Power ON";
		break;
	case ESP_RST_EXT:
		strReason = "External";
		break;
	case ESP_RST_SW:
		strReason = "Software Restart";
		break;
	case ESP_RST_PANIC:
		strReason = "Panic";
		break;
	case ESP_RST_INT_WDT:
		strReason = "Interrupt Watchdog";
		break;
	case ESP_RST_TASK_WDT:
		strReason = "Task Watchdog";
		break;
	case ESP_RST_WDT:
		strReason = "Other Watchdog";
		break;
	case ESP_RST_DEEPSLEEP:
		strReason = "Deep Sleep";
		break;
	case ESP_RST_BROWNOUT:
		strReason = "Brownout";
		break;
	case ESP_RST_SDIO:
		strReason = "SDIO";
		break;
	case ESP_RST_USB:
		strReason = "USB";
		break;
	case ESP_RST_JTAG:
		strReason = "JTAG";
		break;
	case ESP_RST_EFUSE:
		strReason = "E-Fuse";
		break;
	case ESP_RST_PWR_GLITCH:
		strReason = "Power Glitch";
		break;
	case ESP_RST_CPU_LOCKUP:
		strReason = "CPU Lockup";
		break;

	default:
		strReason = "Invalid";
		break;
	}

	log_i("Reset Reason: %d - %s", reason, strReason.c_str());
}

void setup()
{
	Serial.begin(115200);
	delay(1000);
	log_d("==================== Setup Start ====================");

	printResetReason(esp_reset_reason());

	// Watchdog
	wd.init();

	// Heartbeat
	hb.init();
	hb.start();

	// Reset Button with external pullup -> Logic level is inverted
	Reset::init(std::chrono::seconds(3), PIN_BUTTON_RESET, true);

	dc.init();
	sml.init();

	api.init();

	Networking* const net = Networking::init();
	net->enableDfs(40, 160);		// Reduce CPU Frequency when WiFi is not active
	net->addCallbackConnect([]() { hb.pattern(Heartbeat::FAST_1, 3); });
	net->addCallbackApStart([]() { hb.pattern(Heartbeat::FAST_1, 3); });
	net->addCallbackConnect([]() { server.start(); });
	net->addCallbackApStart([]() { server.start(); });
	net->addCallbackDisconnect([]() { server.stop(); });
	net->addCallbackApStop([]() { server.stop(); });

	pinMode(PIN_LED_BUILTIN, OUTPUT);
	digitalWrite(PIN_LED_BUILTIN, HIGH);	// Off

	net->addCallbackConnect([]() { digitalWrite(PIN_LED_BUILTIN, LOW); });
	net->addCallbackDisconnect([]() { digitalWrite(PIN_LED_BUILTIN, HIGH); });

	const auto timeoutClient = ((wd.getTimeout() - std::chrono::seconds(1)) / 3);
	client.init();
	client.setTimeoutConnect(timeoutClient);
	client.setTimeoutHandshake(timeoutClient);
	client.setTimeoutReply(timeoutClient);
	dc.setCallbackSmartmeter([](const std::chrono::system_clock::time_point& tp, const DataSmartMeter& data) { client.callbackSmartmeter(tp, data); });
	dc.setCallbackSystem([](const std::chrono::system_clock::time_point& tp, const DataSystem& data) { client.callbackSystem(tp, data); });

	server.init();
	server.addCallbackSettings([net]() { net->reload(); });
	server.addCallbackSettings([]() { client.reload(); });

	dc.start();

	log_d("==================== Setup End ====================");
}

void loop()
{
	wd.update();		// Watchdog
	hb.update();		// Heartbeat

	sys.update();

	dc.updateDatapoint(DP_UPTIME, std::chrono::duration_cast<std::chrono::seconds>(sys.getUptime()).count(), false);
	dc.updateDatapoint(DP_TEMPERATURE, sys.getTemperature());

	dc.updateDatapoint(DP_MCU_USAGE_1MIN, sys.getMcuUsage1min(), false);
	dc.updateDatapoint(DP_MCU_USAGE_5MIN, sys.getMcuUsage5min(), false);
	dc.updateDatapoint(DP_MCU_USAGE_15MIN, sys.getMcuUsage15min(), false);

	dc.updateDatapoint(DP_RAM_TOTAL_BYTE, sys.getRamHeapSizeTotal(), false);
	dc.updateDatapoint(DP_RAM_USAGE_BYTE, sys.getRamHeapSizeUsed());
	dc.updateDatapoint(DP_RAM_USAGE_PERC, sys.getRamHeapSizePercent());

	Reset::getInstance()->update();

	Networking* const net = Networking::getInstance();
	net->update();

	if (net->isWifiConnected())
		dc.updateDatapoint(DP_WIFI_RSSI, WiFi.RSSI());

	dc.update();		// Data Collector
	sml.update();		// SML Reader

	api.update();		// HTTP API for Server and Client
	client.update();	// HTTP Client
	server.update(); 	// HTTP Server

	// Reduce CPU load when HTTP Server is not running to save power
	if (!server.isRunning())
		delay(100);
}
