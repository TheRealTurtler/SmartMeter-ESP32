#pragma once

#include <ArduinoJson.h>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <chrono>

#include "datacollector.hpp"


class HttpAPI
{
public:
	HttpAPI();
	~HttpAPI() = default;

	void init();
	void update();

	ArduinoJson::JsonDocument buildJsonStatus() const;
	ArduinoJson::JsonDocument buildJsonSmartmeter(const std::chrono::system_clock::time_point& tp, const DataSmartMeter& data, const std::vector<std::string>& vecFilter = { }) const;
	ArduinoJson::JsonDocument buildJsonSystem(const std::chrono::system_clock::time_point& tp, const DataSystem& data, const std::vector<std::string>& vecFilter = { }) const;

	ArduinoJson::JsonDocument buildJsonSettings(const std::vector<std::string>& vecFilter = { }) const;
	ArduinoJson::JsonDocument decodeJsonSettings(const ArduinoJson::JsonDocument& docReceived) const;
};
