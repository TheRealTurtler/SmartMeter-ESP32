#include "httpserver.hpp"
#include "html.hpp"
#include <sstream>


HttpServer::HttpServer(const HttpAPI& api, const DataCollector& dc, uint16_t port):
	m_api(api),
	m_dc(dc),
	m_server(WebServer(port))
{

}

void HttpServer::init()
{
	m_server.on("/", [this]() { this->getHome(); });
	m_server.on("/ping", [this]() { this->getPing(); });

	m_server.on("/api/status", [this]() { this->getStatus(); });
	m_server.on("/api/smartmeter", [this]() { this->getSmartmeter(); });
	m_server.on("/api/system", [this]() { this->getSystem(); });
}

void HttpServer::start()
{
	m_server.begin();
}

void HttpServer::stop()
{
	m_server.stop();
}

void HttpServer::update()
{
	m_server.handleClient();
}

void HttpServer::getHome()
{
	Serial.println("HTTP GET HOME");

	if (Html::isCompressed)
		m_server.sendHeader("Content-Encoding", "gzip");

	m_server.send_P(200, PSTR("text/html; charset=utf-8"), Html::index, Html::index_len);
}

void HttpServer::getPing()
{
	Serial.println("HTTP GET PING");

	m_server.send(200, "text/plain", "Pong");
}

void HttpServer::getStatus()
{
	Serial.println("HTTP GET STATUS");

	const ArduinoJson::JsonDocument doc = m_api.buildJsonStatus();
	sendJsonResponse(200, doc);
}

void HttpServer::getSmartmeter()
{
	Serial.println("HTTP GET SMARTMETER");

	std::vector<std::string> vecFilter;

	if (m_server.hasArg("dp"))
		vecFilter = convertToList(m_server.arg("dp").c_str(), ',');

	const ArduinoJson::JsonDocument doc = m_api.buildJsonSmartmeter(DateTime::currentDateTime(), m_dc.getDataSmartMeter(), vecFilter);
	sendJsonResponse(200, doc);
}

void HttpServer::getSystem()
{
	Serial.println("HTTP GET SYSTEM");

	std::vector<std::string> vecFilter;

	if (m_server.hasArg("dp"))
		vecFilter = convertToList(m_server.arg("dp").c_str(), ',');

	const ArduinoJson::JsonDocument doc = m_api.buildJsonSystem(DateTime::currentDateTime(), m_dc.getDataSystem(), vecFilter);
	sendJsonResponse(200, doc);
}

std::vector<std::string> HttpServer::convertToList(const std::string& str, const char delimiter)
{
	std::vector<std::string> result;

	std::istringstream iss(str);
	std::string token;

	while (std::getline(iss, token, delimiter))
	{
		result.push_back(token);
	}

	return result;
}

void HttpServer::sendJsonResponse(const int httpCode, const ArduinoJson::JsonDocument& doc)
{
	std::string strJson;
	ArduinoJson::serializeJson(doc, strJson);

	m_server.send(httpCode, "application/json", strJson.c_str());
}
