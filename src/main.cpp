#include <Arduino.h>
//#include <WiFiManager.h>
#include <WiFi.h>

#include "components/heartbeat.hpp"
#include "components/watchdog.hpp"
#include "components/timer.hpp"
#include "httpapi.hpp"
#include "smlreader.hpp"

#include "../secrets/wifi.h"


constexpr uint8_t PIN_LED = 8;
constexpr uint8_t PIN_RX = 20;
constexpr uint8_t PIN_TX = 21;

wl_status_t wifiLastState = WL_DISCONNECTED;
const char* HOSTNAME = "ESP32-SmartMeter";

Heartbeat hb(1000, PIN_LED, true);
Watchdog wd(3000);

IEC62065 iec62056(Serial1, 5000, PIN_RX, PIN_TX);
SMLReader sml(Serial1, PIN_RX, PIN_TX);

DataCollector dc(iec62056);
HttpAPI api(dc, 80);

Timer timerWifi;


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

				api.start();
			}
		}
		else if (wifiState == WL_IDLE_STATUS
			|| wifiState == WL_STOPPED
			|| wifiState == WL_CONNECT_FAILED)
		{
			api.stop();

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
	api.init();

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

	wd.update();		// Watchdog
	hb.update();		// Heartbeat

	sml.update();		// SML Reader
	api.update(); 		// HTTP API

	timerWifi.update();
}
