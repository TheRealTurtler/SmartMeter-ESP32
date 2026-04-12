#pragma once

#include <WebServer.h>
#include <string>

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

	void getStatus();
	void getSmartmeter();
};
