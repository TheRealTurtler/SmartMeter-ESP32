#pragma once

#include <WebServer.h>

#include "datacollector.hpp"


class HttpAPI
{
public:
	HttpAPI(const DataCollector& dc, uint16_t port = 80);
	~HttpAPI() = default;

	void init();
	void start();
	void stop();
	void update();

private:
	WebServer m_server;

	const DataCollector& m_dc;

	void getHome();
	void getPing();
	void getStatus();

	void getSmartmeter();
	void getSmartmeterRaw();
};
