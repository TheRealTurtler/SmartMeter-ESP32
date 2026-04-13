#include "httpserver.hpp"

#include <iomanip>
#include <sstream>
#include <ctime>

#include "html.hpp"


HttpServer::HttpServer(const DataCollector& dc, uint16_t port):
	m_server(WebServer(port)),
	m_api(dc, &m_server),
	m_dc(dc)
{

}

void HttpServer::init()
{
	m_server.on("/", [this]() { this->getHome(); });
	m_server.on("/ping", [this]() { this->getPing(); });

	m_api.init();
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
