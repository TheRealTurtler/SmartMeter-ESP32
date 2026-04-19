#include <Arduino.h>
//#include <WiFiManager.h>
#include <WiFi.h>
#include "time.h"

#include "components/heartbeat.hpp"
#include "components/watchdog.hpp"
#include "components/timer.hpp"
#include "web/client/httpclient.hpp"
#include "web/server/httpserver.hpp"
#include "smlreader.hpp"
#include "system.hpp"

// FIXME -> Settings UI
#include "../secrets/wifi.h"


constexpr uint8_t PIN_LED_BUILTIN = 8;
constexpr uint8_t PIN_LED = 6;
constexpr uint8_t PIN_RX = 20;
constexpr uint8_t PIN_TX = 21;

Watchdog wd(5000);
Heartbeat hb(1000, PIN_LED, true);

System sys(wd.getTimeout());

DataCollector dc(DataCollector::AVG_1_MIN);
SMLReader sml(&dc, Serial1, PIN_RX, PIN_TX);

HttpAPI api;
HttpClient client(api, dc);
HttpServer server(api, dc, 80);

// TODO Move to separate file
Timer timerWifi;
wl_status_t wifiLastState = WL_DISCONNECTED;
const char* const HOSTNAME = "ESP32-SmartMeter";


void checkWifi()
{
	// TODO
	if (false)
	{
		/*
		// Initialize WiFiManager
		WiFiManager wifiManager;

		// Auto-connect to Wi-Fi with a portal if not configured
		if (!wifiManager.autoConnect("ESP32-WiFi"))
		{
			Serial.println("Failed to connect and hit timeout");
			ESP.restart();
		}
		*/
	}
	else
	{
		wl_status_t wifiState = WiFi.status();

		//Serial.print("WiFi Status: ");
		//Serial.print(wifiState);

		if (wifiState == WL_CONNECTED)
		{
			const int32_t rssi = WiFi.RSSI();

			if (wifiLastState != wifiState)
			{
				hb.pattern(Heartbeat::FAST_1, 3);
				Serial.print(" --- WiFi connected. IP: ");
				Serial.print(WiFi.localIP());
				Serial.print(" Hostname: ");
				Serial.println(WiFi.getHostname());

				// FIXME NTP Time
				// -> Should be called now and again to keep time accurate
				// -> ESP32 keeps time after reset, but not after power loss
				configTime(2 * 3600, 3600, "pool.ntp.org", "time.nist.gov", "time.google.com");

				server.start();
			}
		}
		else if (wifiState == WL_IDLE_STATUS
			|| wifiState == WL_STOPPED
			|| wifiState == WL_CONNECT_FAILED)
		{
			server.stop();

			//Serial.print(" --- WiFi reconnecting ---");
			WiFi.reconnect();
		}

		wifiLastState = wifiState;
	}
}

void setup()
{
	// Heartbeat
	hb.init();
	hb.start();

	// TODO Debug output
	Serial.begin(9600);

	dc.init();
	sml.init();

	api.init();

	client.init();
	client.setTimeoutConnect((wd.getTimeout() - 1000) / 2);
	client.setTimeoutReply((wd.getTimeout() - 1000) / 2);

	// FIXME -> Settings UI
	//client.setServerHost("http://192.168.178.54");
	client.setServerLocationSmartMeter("/smartmeter/api/upload/smartmeter.php");
	client.setServerLocationSystem("/smartmeter/api/upload/system.php");

	server.init();

	// TODO Start in AP-Mode and let User enter SSID and password
	WiFi.setHostname(HOSTNAME);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	timerWifi.setInterval_s(5);
	timerWifi.setCallback(checkWifi);
	timerWifi.start();

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

	// FIXME
	if (WiFi.status() == WL_CONNECTED)
		dc.updateDatapoint(DP_WIFI_RSSI, WiFi.RSSI());

	dc.updateDatapoint(DP_TEMPERATURE, sys.getTemperature());

	wd.update();		// Watchdog
	hb.update();		// Heartbeat

	dc.update();		// Data Collector
	sml.update();		// SML Reader

	api.update();		// HTTP API for Server and Client
	client.update();	// HTTP Client
	server.update(); 	// HTTP Server

	timerWifi.update();

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
}
