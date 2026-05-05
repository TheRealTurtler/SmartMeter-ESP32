#include <Arduino.h>
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
#include <chrono>


// NOTE: LED_BUILTIN is wrong for ESP32-C3 Super Mini (evaluates to 7 instead of 8)
constexpr uint8_t PIN_LED_BUILTIN = 8;
constexpr uint8_t PIN_LED = 6;
constexpr uint8_t PIN_RX = 20;
constexpr uint8_t PIN_TX = 21;
constexpr uint8_t PIN_BUTTON_RESET = BOOT_PIN;

Watchdog wd(std::chrono::seconds(5));
Heartbeat hb(std::chrono::seconds(1), PIN_LED, true);

System sys(wd.getTimeout());

DataCollector dc(std::chrono::minutes(1));
SMLReader sml(&dc, Serial1, PIN_RX, PIN_TX);

HttpAPI api;
HttpClient client(api, dc);
HttpServer server(api, dc, 80);


void setup()
{
	// Heartbeat
	hb.init();
	hb.start();

	// Reset Button with external pullup -> Logic level is inverted
	Reset::init(std::chrono::seconds(3), PIN_BUTTON_RESET, true);

	// TODO Debug output
	Serial.begin(9600);

	dc.init();
	sml.init();

	api.init();

	client.init();
	client.setTimeoutConnect((wd.getTimeout() - std::chrono::seconds(1)) / 2);
	client.setTimeoutReply((wd.getTimeout() - std::chrono::seconds(1)) / 2);

	server.init();

	Networking* const net = Networking::init();
	net->addCallbackConnect([]() { hb.pattern(Heartbeat::FAST_1, 3); });
	net->addCallbackApStart([]() { hb.pattern(Heartbeat::FAST_1, 3); });
	net->addCallbackConnect([]() { server.start(); });
	net->addCallbackApStart([]() { server.start(); });
	net->addCallbackDisconnect([]() { server.stop(); });
	net->addCallbackApStop([]() { server.stop(); });
	net->addCallbackConnect([]() { client.setEnableUpload(true); });
	net->addCallbackDisconnect([]() { client.setEnableUpload(false); });

	server.addCallbackSettings([net]() { net->reload(); });
	server.addCallbackSettings([]() { client.reload(); });

	// Watchdog
	wd.init();

	dc.start();
}

void loop()
{
	sys.update();

	dc.updateDatapoint(DP_MCU_USAGE_1MIN, sys.getMcuUsage1min(), false);
	dc.updateDatapoint(DP_MCU_USAGE_5MIN, sys.getMcuUsage5min(), false);
	dc.updateDatapoint(DP_MCU_USAGE_15MIN, sys.getMcuUsage15min(), false);

	dc.updateDatapoint(DP_RAM_TOTAL_BYTE, sys.getRamHeapSizeTotal());
	dc.updateDatapoint(DP_RAM_USAGE_BYTE, sys.getRamHeapSizeUsed());
	dc.updateDatapoint(DP_RAM_USAGE_PERC, sys.getRamHeapSizePercent());

	Reset::getInstance()->update();

	Networking* const net = Networking::getInstance();

	net->update();

	if (net->isWifiConnected())
		dc.updateDatapoint(DP_WIFI_RSSI, WiFi.RSSI());

	dc.updateDatapoint(DP_TEMPERATURE, sys.getTemperature());

	wd.update();		// Watchdog
	hb.update();		// Heartbeat

	dc.update();		// Data Collector
	sml.update();		// SML Reader

	api.update();		// HTTP API for Server and Client
	client.update();	// HTTP Client
	server.update(); 	// HTTP Server

	// FIXME remove
	// ---
	static int test = 0;

	if (++test % 1000 == 0)
	{
		test = 0;

		Serial.print("MCU Usage 1min: ");
		Serial.print(sys.getMcuUsage1min());
		Serial.print(" 5min: ");
		Serial.print(sys.getMcuUsage5min());
		Serial.print(" 15min: ");
		Serial.println(sys.getMcuUsage15min());

		Serial.print("MEM Total: ");
		Serial.print(System::getRamHeapSizeTotal());
		Serial.print(" Free: ");
		Serial.print(System::getRamHeapSizeFree());
		Serial.print(" -> Used: ");
		Serial.print(System::getRamHeapSizeUsed());
		Serial.print(" Percent: ");
		Serial.println(System::getRamHeapSizePercent());
	}
	// ---
}
