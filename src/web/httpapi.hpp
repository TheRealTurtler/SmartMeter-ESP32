#pragma once

#include <ArduinoJson.h>
#include <vector>
#include <string>

#include "datacollector.hpp"


class HttpAPI
{
public:
	HttpAPI();
	~HttpAPI() = default;

	void init();
	void update();

	ArduinoJson::JsonDocument buildJsonStatus() const;
	ArduinoJson::JsonDocument buildJsonSmartmeter(const DateTime& dt, const DataSmartMeter& data, const std::vector<std::string>& vecFilter) const;
	ArduinoJson::JsonDocument buildJsonSystem(const DateTime& dt, const DataSystem& data, const std::vector<std::string>& vecFilter) const;
};
