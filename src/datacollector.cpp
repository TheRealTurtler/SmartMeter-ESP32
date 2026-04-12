#include "datacollector.hpp"

DataCollector::DataCollector()
{

}

std::string DataCollector::getJsonStatus() const
{
	std::string result;

	// TODO: Status of Board
	// -> In-/Outputs
	// -> WiFi RSSI
	// -> etc.

	result = "{\"STATUS\":\"OK\"}";

	return result;
}
