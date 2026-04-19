#include "httpapi.hpp"
#include "components/datetime.hpp"


HttpAPI::HttpAPI()
{

}

void HttpAPI::init()
{

}

void HttpAPI::update()
{

}

ArduinoJson::JsonDocument HttpAPI::buildJsonStatus() const
{
	ArduinoJson::JsonDocument doc;
	doc["status"] = "ok";

	return doc;
}

ArduinoJson::JsonDocument HttpAPI::buildJsonSmartmeter(const DateTime& dt, const DataSmartMeter& data, const std::vector<std::string>& vecFilter) const
{
	ArduinoJson::JsonDocument doc;

	if (data.tsLastUpdate > 0)
	{
		doc["status"] = "ok";
		doc["time_dataset"] = dt.toMSecsSinceEpoch();

		const unsigned long tsNow = micros();
		const long tsDelta = tsNow - data.tsLastUpdate;

		const DateTime dtNow = DateTime::currentDateTime();
		const DateTime dtLastUpdate = dtNow.addUSecs(-tsDelta);

		doc["time_last_update"] = dtLastUpdate.toMSecsSinceEpoch();

		for (const auto& [dp, mv] : data.mapData)
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

	return doc;
}

ArduinoJson::JsonDocument HttpAPI::buildJsonSystem(const DateTime& dt, const DataSystem& data, const std::vector<std::string>& vecFilter) const
{
	ArduinoJson::JsonDocument doc;

	doc["status"] = "ok";
	doc["time_dataset"] = dt.toMSecsSinceEpoch();

	doc["time_now"] = DateTime::currentMSecsSinceEpoch();

	for (const auto& [dp, mv] : data.mapData)
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

	return doc;
}

ArduinoJson::JsonDocument HttpAPI::buildJsonSettings(const std::vector<std::string>& vecFilter) const
{
	ArduinoJson::JsonDocument doc;

	doc["status"] = "wip";

	// TODO

	return doc;
}
