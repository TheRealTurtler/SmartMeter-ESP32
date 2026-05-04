#pragma once

#include <WebServer.h>
#include "web/httpapi.hpp"
#include "datacollector.hpp"
#include <functional>


class HttpServer
{
public:
	HttpServer(const HttpAPI& api, const DataCollector& dc, uint16_t port = 80);
	~HttpServer() = default;

	void init();
	void start();
	void stop();
	void update();

	typedef std::function<void(void)> func_cb_settings;

	void addCallbackSettings(const func_cb_settings& func);

private:
	const HttpAPI& m_api;
	const DataCollector& m_dc;

	WebServer m_server;

	bool m_triggerSettingsCallbacks = false;
	std::vector<func_cb_settings> m_vecFuncSettings;

	void getHome();
	void getPing();
	void getOverview();
	void getSettings();

	// API
	void getApiStatus();
	void getApiSmartmeter();
	void getApiSystem();
	void getApiSettings();
	void postApiSettings();

	std::vector<std::string> convertToList(const std::string& str, const char delimiter);

	void sendJsonResponse(const int httpCode, const ArduinoJson::JsonDocument& doc);
};
