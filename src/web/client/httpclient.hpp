#pragma once

#include "web/httpapi.hpp"
#include "datacollector.hpp"
#include <HTTPClient.h>
#include <string>


class HttpClient
{
public:
	HttpClient(const HttpAPI& api, DataCollector& dc);

	void init();
	void update();

	void setServerUrl(const std::string& url) { m_serverUrl = url; }

private:
	const HttpAPI& m_api;

	HTTPClient m_client;

	std::string m_serverUrl = "";

	void callbackSmartmeter(const DateTime& dt, const DataSmartMeter& data);
	void callbackSystem(const DateTime& dt, const DataSystem& data);
};
