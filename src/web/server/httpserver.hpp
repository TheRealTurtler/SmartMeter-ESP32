#pragma once

#include <WebServer.h>
#include <string>

#include "datacollector.hpp"
#include "httpapi.hpp"


class HttpServer
{
public:
	HttpServer(const DataCollector& dc, uint16_t port = 80);
	~HttpServer() = default;

	void init();
	void start();
	void stop();
	void update();

private:
	WebServer m_server;
	HttpAPI m_api;

	const DataCollector& m_dc;

	void getHome();
	void getPing();

	// FIXME
	// Helper methods
	std::string fillTemplate(const std::string& template_html);
	std::string formatValue(double value, uint8_t decimals = 2);
	std::string getCurrentTimestamp();
};
