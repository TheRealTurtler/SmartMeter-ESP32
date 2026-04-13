#include "httpapi.hpp"
#include "html.hpp"
#include <sstream>
#include "time.h"


HttpAPI::HttpAPI(const DataCollector& dc, WebServer* const server):
	m_server(server),
	m_dc(dc)
{

}

void HttpAPI::init()
{
	m_server->on("/api/status", [this]() { this->getStatus(); });
	m_server->on("/api/smartmeter", [this]() { this->getSmartmeter(); });
	m_server->on("/api/system", [this]() { this->getSystem(); });
}

std::vector<std::string> HttpAPI::convertToList(const std::string& str, const char delimiter)
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

void HttpAPI::getStatus()
{
	Serial.println("HTTP GET STATUS");

	ArduinoJson::JsonDocument doc;

	doc["status"] = "ok";

	sendJsonResponse(200, doc);
}

void HttpAPI::getSmartmeter()
{
	Serial.println("HTTP GET SMARTMETER");

	ArduinoJson::JsonDocument doc;
	std::vector<std::string> vecFilter;

	if (m_server->hasArg("dp"))
		vecFilter = convertToList(m_server->arg("dp").c_str(), ',');

	const int64_t tsLastUpdate = m_dc.getTsLastUpdateSmartmeter();

	if (tsLastUpdate >= 0)
	{
		doc["status"] = "ok";

		const std::map<DATA_POINT_SMARTMETER, MeasuredValue>& mapDataPoints = m_dc.getDataPointsSmartmeter();

		const int64_t tsNow = micros();
		const int64_t tsDelta = tsNow - tsLastUpdate;

		const int64_t timeNow = getUnixTime_us();
		const int64_t timeLastUpdate = timeNow - tsDelta;

		doc["last_update"] = (timeLastUpdate / 1000000);

		for (const auto& [dp, mv] : mapDataPoints)
		{
			if (gc_mapDataPointInfoSmartmeter.contains(dp))
			{
				const std::string& key = gc_mapDataPointInfoSmartmeter.at(dp);

				// If filter is set, then only include datapoints that are in the filter list
				if (!vecFilter.empty() && std::find(vecFilter.begin(), vecFilter.end(), key) == vecFilter.end())
					continue;

				ArduinoJson::JsonObject obj = doc[key].to<JsonObject>();

				if (dp == DP_ACTIVE_ENERGY_IMPORT || dp == DP_ACTIVE_ENERGY_EXPORT)
					obj["abs"] = mv.getValueNow();
				else
				{
					obj["now"] = mv.getValueNow();
					obj["avg"] = mv.getValueAverage();
				}
			}
		}
	}
	else
	{
		doc["status"] = "no_data";
	}

	sendJsonResponse(200, doc);
}

void HttpAPI::getSystem()
{
	Serial.println("HTTP GET SYSTEM");

	ArduinoJson::JsonDocument doc;
	std::vector<std::string> vecFilter;

	if (m_server->hasArg("dp"))
		vecFilter = convertToList(m_server->arg("dp").c_str(), ',');

	doc["status"] = "ok";
	doc["time"] = getUnixTime_s();

	const std::map<DATA_POINT_SYSTEM, MeasuredValue>& mapDataPoints = m_dc.getDataPointsSystem();

	for (const auto& [dp, mv] : mapDataPoints)
	{
		if (gc_mapDataPointInfoSystem.contains(dp))
		{
			const std::string& key = gc_mapDataPointInfoSystem.at(dp);

			// If filter is set, then only include datapoints that are in the filter list
			if (!vecFilter.empty() && std::find(vecFilter.begin(), vecFilter.end(), key) == vecFilter.end())
				continue;

			ArduinoJson::JsonObject obj = doc[key].to<JsonObject>();
			obj["now"] = mv.getValueNow();
			obj["avg"] = mv.getValueAverage();
		}
	}

	sendJsonResponse(200, doc);
}

void HttpAPI::sendJsonResponse(const int httpCode, const ArduinoJson::JsonDocument& doc)
{
	std::string strJson;
	ArduinoJson::serializeJson(doc, strJson);

	m_server->send(httpCode, "application/json", strJson.c_str());
}

int64_t HttpAPI::getUnixTime_us()
{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);

	return static_cast<int64_t>(tv_now.tv_sec) * 1000000L + tv_now.tv_usec;
}
