#pragma once

#include <WiFi.h>
#include <functional>
#include <vector>
#include <chrono>
#include "components/timer.hpp"


class Networking
{
public:
	static Networking* getInstance() { return m_instance; }
	static Networking* init();

	struct SettingsNetwork
	{
		std::string ssid;
		std::string password;
		std::string mode;
		std::string hostname;
		std::string ip;
		std::string subnet;
		std::string gateway;
		std::string dns_1;
		std::string dns_2;
	};

	struct SettingsNtp
	{
		std::string ntp_1;
		std::string ntp_2;
		std::string ntp_3;
		long offsetGmt;
		long offsetDst;
	};

	void reload();

	void update();

	bool isConfigured() { return m_configured; }
	bool isWifiConnected() { return m_wifiStateLast; }

	typedef std::function<void(void)> func_cb_connect;
	typedef std::function<void(void)> func_cb_disconnect;

	typedef std::function<void(void)> func_cb_ap_start;
	typedef std::function<void(void)> func_cb_ap_stop;

	void addCallbackConnect(const func_cb_connect& func);
	void addCallbackDisconnect(const func_cb_disconnect& func);

	void addCallbackApStart(const func_cb_ap_start& func);
	void addCallbackApStop(const func_cb_ap_stop& func);

	static bool isValidIPv4(const std::string& ip);

	static SettingsNetwork loadSettingsNetwork();
	static void saveSettingsNetwork(const SettingsNetwork& settings);
	static bool validateNetworkSettings(const SettingsNetwork& settings);

	static SettingsNtp loadSettingsNtp();
	static void saveSettingsNtp(const SettingsNtp& settings);
	static bool validateNtpSettings(const SettingsNtp& settings);

private:
	Networking();
	~Networking();

	static Networking* m_instance;

	Timer m_timerWifi;

	wl_status_t m_wifiStateLast = WL_DISCONNECTED;

	std::vector<func_cb_connect> m_vecFuncConnect;
	std::vector<func_cb_disconnect> m_vecFuncDisonnect;

	std::vector<func_cb_ap_start> m_vecFuncApStart;
	std::vector<func_cb_ap_stop> m_vecFuncApStop;

	bool m_configured = false;
	bool m_started = false;

	std::chrono::steady_clock::time_point m_timeLastSyncNtp;
	std::chrono::hours m_intervalSyncNtp;

	void updateWifi();
	void updateWifiStation();
	void updateWifiAp();

	void checkNtp();
	void syncNtp();

	void onConnect();
	void onDisconnect();

	void onApStart();
	void onApStop();
};
