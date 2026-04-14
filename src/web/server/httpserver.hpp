#pragma once

#include <WebServer.h>
#include "web/httpapi.hpp"
#include "datacollector.hpp"


class HttpServer
{
public:
	HttpServer(const HttpAPI& api, const DataCollector& dc, uint16_t port = 80);
	~HttpServer() = default;

	void init();
	void start();
	void stop();
	void update();

private:
	const HttpAPI& m_api;
	const DataCollector& m_dc;

	WebServer m_server;

	void getHome();
	void getPing();

	// API
	void getStatus();
	void getSmartmeter();
	void getSystem();

	std::vector<std::string> convertToList(const std::string& str, const char delimiter);

	void sendJsonResponse(const int httpCode, const ArduinoJson::JsonDocument& doc);
};
