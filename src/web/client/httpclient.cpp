#include "httpclient.hpp"
#include "system.hpp"

#include "Arduino.h"


HttpClient::HttpClient(const HttpAPI& api, DataCollector& dc):
	m_api(api)
{
	auto cbSmartmeter = [this](const DateTime& dt, const DataSmartMeter& data) { this->callbackSmartmeter(dt, data); };
	auto cbSystem = [this](const DateTime& dt, const DataSystem& data) { this->callbackSystem(dt, data); };

	dc.setCallbackSmartmeter(cbSmartmeter);
	dc.setCallbackSystem(cbSystem);
}

void HttpClient::init()
{
	m_tsLast = millis();
}

void HttpClient::update()
{
	const uint32_t timeDiff = (millis() - m_tsLast);

	if (timeDiff < m_delayNext)
		return;

	if (m_serverHost == "")
	{
		m_delayNext = m_delayRetry;
		m_tsLast = millis();
	}

	// In order to avoid watchdog timeouts only one request is sent per update()
	// To avoid spamming the server, a small delay is set after every request
	// -> This delay can be configured, depending on if the request was successful or if there was an error

	if (!handleResult(uploadSmartMeter()))
	{
		m_tsLast = millis();
		return;
	}

	if (!handleResult(uploadSystem()))
	{
		m_tsLast = millis();
		return;
	}

	m_tsLast = millis();
}

void HttpClient::callbackSmartmeter(const DateTime& dt, const DataSmartMeter& data)
{
	// Remove oldest data if memory gets full
	if (System::getRamHeapSizePercent() > 90.0f && !m_mapDataSmartMeter.empty())
	{
		const DateTime dt = m_mapDataSmartMeter.cbegin()->first;

		Serial.print("SM - Removing Data from: ");
		Serial.println(dt.toString("%Y-%m-%d %H:%M:%S").c_str());

		m_mapDataSmartMeter.erase(dt);

		if (m_mapDataSystem.contains(dt))
			m_mapDataSystem.erase(dt);
	}

	m_mapDataSmartMeter[dt] = data;
}

void HttpClient::callbackSystem(const DateTime& dt, const DataSystem& data)
{
	// Remove oldest data if memory gets full
	if (System::getRamHeapSizePercent() > 90.0f && !m_mapDataSystem.empty())
	{
		const DateTime dt = m_mapDataSystem.cbegin()->first;

		Serial.print("SYS - Removing Data from: ");
		Serial.println(dt.toString("%Y-%m-%d %H:%M:%S").c_str());

		m_mapDataSystem.erase(dt);

		if (m_mapDataSmartMeter.contains(dt))
			m_mapDataSmartMeter.erase(dt);
	}

	m_mapDataSystem[dt] = data;
}

int8_t HttpClient::uploadSmartMeter()
{
	int8_t result = 0;

	if (!m_mapDataSmartMeter.empty())
	{
		const auto& it = m_mapDataSmartMeter.cbegin();

		const std::string url = (m_serverHost + m_serverLocationSmartMeter);
		const ArduinoJson::JsonDocument doc = m_api.buildJsonSmartmeter(it->first, it->second);

		if (uploadJson(url, doc))
		{
			result = 1;
			m_mapDataSmartMeter.erase(it);
		}
		else
			result = -1;
	}

	return result;
}

int8_t HttpClient::uploadSystem()
{
	int8_t result = 0;

	if (!m_mapDataSystem.empty())
	{
		const auto& it = m_mapDataSystem.cbegin();

		const std::string url = (m_serverHost + m_serverLocationSystem);
		const ArduinoJson::JsonDocument doc = m_api.buildJsonSystem(it->first, it->second);

		if (uploadJson(url, doc))
		{
			m_mapDataSystem.erase(it);
			result = 1;
		}
		else
			result = -1;
	}

	return result;
}

bool HttpClient::uploadJson(const std::string& url, const ArduinoJson::JsonDocument& doc)
{
	bool result = false;

	Serial.println("Sending HTTP Request to: ");
	Serial.println(url.c_str());

	std::string strJson;
	ArduinoJson::serializeJson(doc, strJson);

	m_client.begin(url.c_str());
	m_client.addHeader("Content-Type", "application/json");

	m_client.setConnectTimeout(m_timeoutConnect);
	m_client.setTimeout(m_timeoutReply);

	const int httpResponseCode = m_client.POST(strJson.c_str());

	if (httpResponseCode > 0)
	{
		Serial.print("HTTP Response Code: ");
		Serial.println(httpResponseCode);

		if (m_client.getSize() > 0)
		{
			const String payload = m_client.getString();
			Serial.print("Payload: ");
			Serial.println(payload);
		}

		if (httpResponseCode >= 200 && httpResponseCode < 300)
			result = true;
	}
	else
	{
		Serial.print("Error sending HTTP Request: ");
		Serial.println(httpResponseCode);
	}

	m_client.end();

	return result;
}

bool HttpClient::handleResult(int8_t resultUpload)
{
	bool result = true;

	if (resultUpload > 0)
	{
		m_delayNext = m_delayRequest;
		result = false;
	}
	else if (resultUpload < 0)
	{
		m_delayNext = m_delayRetry;
		result = false;
	}

	return result;
}
