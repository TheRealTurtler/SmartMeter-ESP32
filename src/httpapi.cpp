#include "httpapi.hpp"

HttpAPI::HttpAPI(const DataCollector& dc, uint16_t port):
	m_server(WebServer(port)),
	m_dc(dc)
{

}

void HttpAPI::init()
{
	m_server.on("/", [this]() { this->getHome(); });
	m_server.on("/ping", [this]() { this->getPing(); });
	m_server.on("/status", [this]() { this->getStatus(); });
	m_server.on("/smartmeter", [this]() { this->getSmartmeter(); });
	m_server.on("/smartmeter/raw", [this]() { this->getSmartmeterRaw(); });
}

void HttpAPI::start()
{
	m_server.begin();
}

void HttpAPI::stop()
{
	m_server.stop();
}

void HttpAPI::update()
{
	m_server.handleClient();
}

void HttpAPI::getHome()
{
	Serial.println("GET HOME");
	m_server.send(200, "text/plain", "Hello from ESP32-C3!");
}

void HttpAPI::getPing()
{
	Serial.println("GET PING");
	m_server.send(200, "text/plain", "Pong");
}

void HttpAPI::getStatus()
{
	Serial.println("GET STATUS");

	const std::string strData = m_dc.getJsonStatus();
	m_server.send(200, "application/json", strData.c_str());
}

void HttpAPI::getSmartmeter()
{
	Serial.println("GET Smartmeter");

	const std::string strData = m_dc.getJsonIEC62065();
	m_server.send(200, "application/json", strData.c_str());
}

void HttpAPI::getSmartmeterRaw()
{
	Serial.println("GET RAW");

	const std::string& strData = m_dc.getRawIEC62065();
	m_server.send(200, "text/plain", strData.c_str());
}
