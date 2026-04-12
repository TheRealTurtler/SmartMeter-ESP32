#include "httpapi.hpp"
#include "html.hpp"


HttpAPI::HttpAPI(const DataCollector& dc, WebServer* const server):
	m_server(server),
	m_dc(dc)
{

}

void HttpAPI::init()
{
	m_server->on("/api/status", [this]() { this->getStatus(); });
	m_server->on("/api/smartmeter", [this]() { this->getSmartmeter(); });
}

void HttpAPI::getStatus()
{
	Serial.println("HTTP GET STATUS");

	// FIXME
	const std::string strData = m_dc.getJsonStatus();
	m_server->send(200, "application/json", strData.c_str());
}

void HttpAPI::getSmartmeter()
{
	Serial.println("HTTP GET SMARTMETER");

	// FIXME
	const std::string strData = m_dc.getJsonStatus();
	m_server->send(200, "application/json", strData.c_str());
}
