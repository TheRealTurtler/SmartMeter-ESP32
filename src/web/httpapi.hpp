#pragma once

#include <WebServer.h>
#include <string>
#include <ArduinoJson.h>

#include "datacollector.hpp"


class HttpAPI
{
public:
	HttpAPI(const DataCollector& dc, WebServer* const server);
	~HttpAPI() = default;

	void init();

private:
	WebServer* const m_server;

	const DataCollector& m_dc;

	std::vector<std::string> convertToList(const std::string& str, const char delimiter);

	void getStatus();
	void getSmartmeter();
	void getSystem();

	void sendJsonResponse(const int httpCode, const ArduinoJson::JsonDocument& doc);

	int64_t getUnixTime_us();
	int64_t getUnixTime_ms() { return getUnixTime_us() / 1000; }
	int64_t getUnixTime_s() { return getUnixTime_ms() / 1000; }
};
