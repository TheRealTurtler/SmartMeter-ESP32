#pragma once

#include "web/httpapi.hpp"
#include "datacollector.hpp"
#include <HTTPClient.h>
#include <string>
#include <chrono>


class HttpClient
{
public:
	HttpClient(const HttpAPI& api, DataCollector& dc);

	struct Settings
	{
		bool enable = false;
		std::string serverHost = "";
		std::string serverLocationSmartMeter = "/";
		std::string serverLocationSystem = "/";
		uint16_t batchSize = 1;
		bool disbaleWifi = false;
	};

	void init();
	void reload();
	void update();

	void setEnableUpload(bool enable) { m_enableUpload = enable; }

	void setTimeoutConnect(const std::chrono::milliseconds& timeout) { m_timeoutConnect = timeout; }
	void setTimeoutReply(const std::chrono::milliseconds& timeout) { m_timeoutReply = timeout; }

	void setDelayRequest(const std::chrono::milliseconds& delay) { m_delayRequest = delay; }
	void setDelayRetry(const std::chrono::milliseconds& delay) { m_delayRetry = delay; }

	static Settings loadSettings();
	static void saveSettings(const Settings& settings);
	static bool validateSettings(const Settings& settings);

private:
	const HttpAPI& m_api;

	HTTPClient m_client;

	Settings m_settings;

	bool m_enableUpload = false;
	uint16_t m_batchCounter = 0;

	std::chrono::steady_clock::time_point m_timeLast;
	std::chrono::milliseconds m_delayNext = std::chrono::seconds(1);

	std::chrono::milliseconds m_delayRequest = std::chrono::milliseconds(500);
	std::chrono::milliseconds m_delayRetry = std::chrono::seconds(5);

	std::chrono::milliseconds m_timeoutConnect = std::chrono::seconds(1);;
	std::chrono::milliseconds m_timeoutReply = std::chrono::seconds(1);

	std::map<std::chrono::system_clock::time_point, DataSmartMeter> m_mapDataSmartMeter;
	std::map<std::chrono::system_clock::time_point, DataSystem> m_mapDataSystem;

	void callbackSmartmeter(const std::chrono::system_clock::time_point& tp, const DataSmartMeter& data);
	void callbackSystem(const std::chrono::system_clock::time_point& tp, const DataSystem& data);

	int8_t uploadSmartMeter();
	int8_t uploadSystem();

	bool uploadJson(const std::string& url, const ArduinoJson::JsonDocument& doc);

	bool handleResult(int8_t resultUpload);
};
