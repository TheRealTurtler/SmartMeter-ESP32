#include "httpclient.hpp"
#include "Arduino.h"
#include "system.hpp"
#include "config/config_pushapi.hpp"


HttpClient::HttpClient(const HttpAPI& api, DataCollector& dc):
	m_api(api)
{
	auto cbSmartmeter = [this](const std::chrono::system_clock::time_point& tp, const DataSmartMeter& data) { this->callbackSmartmeter(tp, data); };
	auto cbSystem = [this](const std::chrono::system_clock::time_point& tp, const DataSystem& data) { this->callbackSystem(tp, data); };

	dc.setCallbackSmartmeter(cbSmartmeter);
	dc.setCallbackSystem(cbSystem);
}

void HttpClient::init()
{
	m_timeLast = std::chrono::steady_clock::now();
	reload();
}

void HttpClient::reload()
{
	const Settings settings = loadSettings();

	if (validateSettings(settings))
	{
		m_settings = settings;
	}
	else
	{
		m_settings.enable = false;
		m_settings.serverHost = "";
		m_settings.serverLocationSmartMeter = "/";
		m_settings.serverLocationSystem = "/";
	}

	if (!m_settings.enable)
	{
		m_mapDataSmartMeter.clear();
		m_mapDataSystem.clear();
	}
}

void HttpClient::update()
{
	const auto timeNow = std::chrono::steady_clock::now();
	const auto timeDiff = (timeNow - m_timeLast);

	if (timeDiff < m_delayNext)
		return;

	if (!m_settings.enable || m_settings.serverHost == "" || !m_enableUpload)
	{
		m_delayNext = m_delayRetry;
		m_timeLast = timeNow;
		return;
	}

	// In order to avoid watchdog timeouts only one request is sent per update()
	// To avoid spamming the server, a small delay is set after every request
	// -> This delay can be configured, depending on if the request was successful or if there was an error

	if (!handleResult(uploadSmartMeter()))
	{
		m_timeLast = std::chrono::steady_clock::now();
		return;
	}

	if (!handleResult(uploadSystem()))
	{
		m_timeLast = std::chrono::steady_clock::now();
		return;
	}

	m_timeLast = std::chrono::steady_clock::now();
}

HttpClient::Settings HttpClient::loadSettings()
{
	Config_PushAPI cfg;

	Settings settings;
	settings.enable = (cfg.getConfig(Config_PushAPI::ENABLE) == "true");
	settings.serverHost = cfg.getConfig(Config_PushAPI::SERVER_HOST);
	settings.serverLocationSmartMeter = cfg.getConfig(Config_PushAPI::SERVER_PATH_SMARTMETER);
	settings.serverLocationSystem = cfg.getConfig(Config_PushAPI::SERVER_PATH_SYSTEM);

	return settings;
}

void HttpClient::saveSettings(const Settings& settings)
{
	Config_PushAPI cfg;

	cfg.setConfig(Config_PushAPI::ENABLE, (settings.enable ? "true" : "false"));
	cfg.setConfig(Config_PushAPI::SERVER_HOST, settings.serverHost);
	cfg.setConfig(Config_PushAPI::SERVER_PATH_SMARTMETER, settings.serverLocationSmartMeter);
	cfg.setConfig(Config_PushAPI::SERVER_PATH_SYSTEM, settings.serverLocationSystem);
}

bool HttpClient::validateSettings(const Settings& settings)
{
	bool result = true;

	if (settings.enable)
	{
		if (settings.serverHost.empty())
			result = false;

		if (settings.serverLocationSmartMeter.empty())
			result = false;

		if (settings.serverLocationSystem.empty())
			result = false;
	}

	return result;
}

void HttpClient::callbackSmartmeter(const std::chrono::system_clock::time_point& tp, const DataSmartMeter& data)
{
	if (!m_settings.enable)
		return;

	// Remove oldest data if memory gets full
	if (System::getRamHeapSizePercent() > 90.0f && !m_mapDataSmartMeter.empty())
	{
		log_w("SM - Removing Oldest Data");

		const auto tpData = m_mapDataSmartMeter.cbegin()->first;
		m_mapDataSmartMeter.erase(tpData);

		if (m_mapDataSystem.contains(tpData))
			m_mapDataSystem.erase(tpData);
	}

	m_mapDataSmartMeter[tp] = data;
}

void HttpClient::callbackSystem(const std::chrono::system_clock::time_point& tp, const DataSystem& data)
{
	if (!m_settings.enable)
		return;

	// Remove oldest data if memory gets full
	if (System::getRamHeapSizePercent() > 90.0f && !m_mapDataSystem.empty())
	{
		log_w("SYS - Removing Oldest Data");

		const auto tpData = m_mapDataSystem.cbegin()->first;
		m_mapDataSystem.erase(tpData);

		if (m_mapDataSmartMeter.contains(tpData))
			m_mapDataSmartMeter.erase(tpData);
	}

	m_mapDataSystem[tp] = data;
}

int8_t HttpClient::uploadSmartMeter()
{
	int8_t result = 0;

	if (!m_mapDataSmartMeter.empty())
	{
		const auto& it = m_mapDataSmartMeter.cbegin();

		const std::string url = (m_settings.serverHost + m_settings.serverLocationSmartMeter);
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

		const std::string url = (m_settings.serverHost + m_settings.serverLocationSystem);
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

	log_d("Sending HTTP Request to: %s", url.c_str());

	std::string strJson;
	ArduinoJson::serializeJson(doc, strJson);

	m_client.begin(url.c_str());
	m_client.addHeader("Content-Type", "application/json");

	m_client.setConnectTimeout(m_timeoutConnect.count());
	m_client.setTimeout(m_timeoutReply.count());

	const int httpResponseCode = m_client.POST(strJson.c_str());

	if (httpResponseCode > 0)
	{
		log_d("HTTP Response Code: %d", httpResponseCode);

		if (m_client.getSize() > 0)
		{
			const String payload = m_client.getString();
			log_d("Payload: %s", payload.c_str());
		}

		if (httpResponseCode >= 200 && httpResponseCode < 300)
			result = true;
	}
	else
	{
		log_e("Error sending HTTP Request: %d", httpResponseCode);
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
