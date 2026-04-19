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
	m_server.on("/", HTTP_GET, [this]() { this->getHome(); });
	m_server.on("/ping", HTTP_GET, [this]() { this->getPing(); });
	m_server.on("/settings", HTTP_GET, [this]() { this->getSettings(); });

	m_server.on("/api/status", HTTP_GET, [this]() { this->getApiStatus(); });
	m_server.on("/api/smartmeter", HTTP_GET, [this]() { this->getApiSmartmeter(); });
	m_server.on("/api/system", HTTP_GET, [this]() { this->getApiSystem(); });
	m_server.on("/api/settings", HTTP_GET, [this]() { this->getApiSettings(); });
	m_server.on("/api/settings", HTTP_POST, [this]() { this->postApiSettings(); });
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

void HttpServer::getSettings()
{
	Serial.println("HTTP GET SETTINGS");

	if (Html::isCompressed)
		m_server.sendHeader("Content-Encoding", "gzip");

	m_server.send_P(200, PSTR("text/html; charset=utf-8"), Html::settings, Html::settings_len);
}

void HttpServer::getApiStatus()
{
	Serial.println("HTTP API GET STATUS");

	const ArduinoJson::JsonDocument doc = m_api.buildJsonStatus();
	sendJsonResponse(200, doc);
}

void HttpServer::getApiSmartmeter()
{
	Serial.println("HTTP API GET SMARTMETER");

	std::vector<std::string> vecFilter;

	if (m_server.hasArg("filter"))
		vecFilter = convertToList(m_server.arg("filter").c_str(), ',');

	const ArduinoJson::JsonDocument doc = m_api.buildJsonSmartmeter(DateTime::currentDateTime(), m_dc.getDataSmartMeter(), vecFilter);
	sendJsonResponse(200, doc);
}

void HttpServer::getApiSystem()
{
	Serial.println("HTTP API GET SYSTEM");

	std::vector<std::string> vecFilter;

	if (m_server.hasArg("filter"))
		vecFilter = convertToList(m_server.arg("filter").c_str(), ',');

	const ArduinoJson::JsonDocument doc = m_api.buildJsonSystem(DateTime::currentDateTime(), m_dc.getDataSystem(), vecFilter);
	sendJsonResponse(200, doc);
}

void HttpServer::getApiSettings()
{
	Serial.println("HTTP API GET SYSTEM");

	std::vector<std::string> vecFilter;

	if (m_server.hasArg("filter"))
		vecFilter = convertToList(m_server.arg("filter").c_str(), ',');

	const ArduinoJson::JsonDocument doc = m_api.buildJsonSettings(vecFilter);
	sendJsonResponse(200, doc);
}

void HttpServer::postApiSettings()
{
	Serial.println("HTTP API POST SYSTEM");

	// TODO

	ArduinoJson::JsonDocument doc;
	doc["status"] = "wip";

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
