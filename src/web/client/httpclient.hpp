#pragma once

#include "web/httpapi.hpp"
#include "datacollector.hpp"
#include <HTTPClient.h>
#include <string>


class HttpClient
{
public:
	HttpClient(const HttpAPI& api, DataCollector& dc);

	struct Settings
	{
		bool enable = false;
		std::string serverHost;
		std::string serverLocationSmartMeter;
		std::string serverLocationSystem;
	};

	void init();
	void reload();
	void update();

	void setTimeoutConnect(const int32_t& timeout) { m_timeoutConnect = timeout; }
	void setTimeoutReply(uint16_t timeout) { m_timeoutReply = timeout; }

	void setDelayRequest(const uint32_t& delay) { m_delayRequest = delay; }
	void setDelayRetry(const uint32_t& delay) { m_delayRetry = delay; }

	static Settings loadSettings();
	static void saveSettings(const Settings& settings);
	static bool validateSettings(const Settings& settings);

private:
	const HttpAPI& m_api;

	HTTPClient m_client;

	Settings m_settings;

	uint32_t m_tsLast = 0;
	uint32_t m_delayNext = 1000;

	uint32_t m_delayRequest = 500;
	uint32_t m_delayRetry = (5 * 1000);

	int32_t m_timeoutConnect = (1 * 1000);
	uint16_t m_timeoutReply = (1 * 1000);

	std::map<DateTime, DataSmartMeter> m_mapDataSmartMeter;
	std::map<DateTime, DataSystem> m_mapDataSystem;

	void callbackSmartmeter(const DateTime& dt, const DataSmartMeter& data);
	void callbackSystem(const DateTime& dt, const DataSystem& data);

	int8_t uploadSmartMeter();
	int8_t uploadSystem();

	bool uploadJson(const std::string& url, const ArduinoJson::JsonDocument& doc);

	bool handleResult(int8_t resultUpload);
};
