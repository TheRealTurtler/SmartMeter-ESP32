#include <Arduino.h>
//#include <WiFiManager.h>
#include <WiFi.h>

#include "components/heartbeat.hpp"
#include "components/watchdog.hpp"
#include "components/timer.hpp"
#include "web/server/httpserver.hpp"
#include "smlreader.hpp"
#include "time.h"

#include "../secrets/wifi.h"


constexpr uint8_t PIN_LED_BUILTIN = 8;
constexpr uint8_t PIN_LED = 6;
constexpr uint8_t PIN_RX = 20;
constexpr uint8_t PIN_TX = 21;

Watchdog wd(3000);
Heartbeat hb(1000, PIN_LED, true);

DataCollector dc;
SMLReader sml(&dc, Serial1, PIN_RX, PIN_TX);
HttpServer server(dc, 80);

// TODO Move to separate file
Timer timerWifi;
wl_status_t wifiLastState = WL_DISCONNECTED;
const char* HOSTNAME = "ESP32-SmartMeter";

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

			//Serial.print(" RSSI: ");
			//Serial.print(rssi);

			if (wifiLastState != wifiState)
			{
				hb.pattern(Heartbeat::FAST_1, 3);
				Serial.print(" --- WiFi connected. IP: ");
				Serial.print(WiFi.localIP());
				Serial.print(" Hostname: ");
				Serial.print(WiFi.getHostname());

				// FIXME NTP Time
				// -> Should be called now and again to keep time accurate
				// -> ESP32 keeps time after reset, but not after power loss
				configTime(0, 3600, "pool.ntp.org", "time.nist.gov", "time.google.com");

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

		//Serial.println();

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

	sml.init();
	server.init();

	// TODO Start in AP-Mode and let User enter SSID and password
	WiFi.setHostname(HOSTNAME);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	timerWifi.setInterval_s(5);
	timerWifi.setCallback(checkWifi);
	timerWifi.start();

	// Watchdog
	wd.init();
}

void loop()
{
	static unsigned long timeLast = micros();

	const unsigned long timeNow = micros();
	const unsigned long timeDiff = (timeNow - timeLast);
	timeLast = timeNow;

	// MCU Usage calculated from last loop time and Watchdog timeout
	// -> 100% means that the loop() takes as long as the Watchdog timeout, which would cause a reset
	const float mcuUsage = (100.0f * timeDiff) / (wd.getTimeout() * 1000.0f);

	dc.updateDatapoint(DP_MCU_USAGE, mcuUsage);

	if (WiFi.status() == WL_CONNECTED)
		dc.updateDatapoint(DP_WIFI_RSSI, WiFi.RSSI());

	wd.update();		// Watchdog
	hb.update();		// Heartbeat

	sml.update();		// SML Reader
	dc.update();		// Data Collector (calculate power factor, L-L voltage etc.)
	server.update(); 	// HTTP Server

	timerWifi.update();
}
