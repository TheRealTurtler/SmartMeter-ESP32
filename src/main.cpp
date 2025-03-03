#include <Arduino.h>
//#include <WiFiManager.h>
#include <WiFi.h>

#include "components/heartbeat.hpp"
#include "components/watchdog.hpp"
#include "components/timer.hpp"
#include "httpapi.hpp"
#include "iec62056.hpp"

#include "../secrets/wifi.h"


constexpr uint8_t PIN_LED = 8;
constexpr uint8_t PIN_RX = 20;
constexpr uint8_t PIN_TX = 21;

wl_status_t wifiLastState = WL_DISCONNECTED;
const char* HOSTNAME = "ESP32-SmartMeter";

Heartbeat hb(1000, PIN_LED, true);
Watchdog wd;

IEC62065 iec62056(Serial1, 5000, PIN_RX, PIN_TX);
HttpAPI api(80);

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

		Serial.print("WiFi Status: ");
		Serial.print(wifiState);

		if (wifiState == WL_CONNECTED)
		{
			const int32_t rssi = WiFi.RSSI();

			Serial.print(" RSSI: ");
			Serial.print(rssi);

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

			Serial.print(" --- WiFi reconnecting ---");
			WiFi.reconnect();
		}

		Serial.println();

		wifiLastState = wifiState;
	}
}

void setup()
{
	// Heartbeat
	hb.init();
	hb.start();

	Serial.begin(9600);

	iec62056.init();
	api.init();

	// TODO: Start in AP-Mode and let User enter SSID and password
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
	// TODO: CPU-Usage
	// -> 100% = Watchdog Timeout
	const unsigned long timeNow = millis();

	// Reset Watchdog
	wd.update();

	hb.update();

	iec62056.update();
	api.update();

	timerWifi.update();
}
