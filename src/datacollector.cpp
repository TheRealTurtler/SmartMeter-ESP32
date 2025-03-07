#include "datacollector.hpp"

DataCollector::DataCollector(const IEC62065& iec):
	m_iec(iec)
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

std::string DataCollector::getJsonIEC62065() const
{
	std::string result;

	// TODO
	const std::string strManuf = m_iec.getManufacturer();
	const std::string strIdent = m_iec.getIdentifier();

	// TODO
	result = "{}";

	return result;
}
