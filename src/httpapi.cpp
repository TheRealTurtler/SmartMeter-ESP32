#include "httpapi.hpp"

HttpAPI::HttpAPI(uint16_t port):
	m_server(WebServer(port))
{

}

void HttpAPI::init()
{
	m_server.on("/", [this]() { this->getHome(); });
	m_server.on("/ping", [this]() { this->getPing(); });
	m_server.on("/status", [this]() { this->getStatus(); });
	m_server.on("/raw", [this]() { this->getRaw(); });
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

	// TODO: Status of Board
	// -> In-/Outputs
	// -> WiFi
	// -> etc.
	std::string strData = "{\"STATUS\":\"OK\"}";

	// TODO: JSON
	m_server.send(200, "application/json", strData.c_str());
}

void HttpAPI::getRaw()
{
	Serial.println("GET RAW");

	// TODO
	std::string strData = "{\"RAW\":\"DATA\"}";

	// TODO: JSON
	m_server.send(200, "application/json", strData.c_str());
}
