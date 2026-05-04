#include "config_network.hpp"


const std::unordered_map<Config_Network::CONFIG_KEY, Config_Network::ConfigData> Config_Network::m_umapConfigData =
{
	{ SSID,						{"ssid",		""} },
	{ PASSWORD,					{"password",	""} },
	{ MODE,						{"mode",		"dhcp"} },
	{ HOSTNAME,					{"hostname",	"ESP32-SmartMeter"} },
	{ IP,						{"ip",			""} },
	{ SUBNET,					{"subnet",		""} },
	{ GATEWAY,					{"gateway",		""} },
	{ DNS_1,					{"dns_1",		""} },
	{ DNS_2,					{"dns_2",		""} },
};

Config_Network::Config_Network():
	Config("network")
{

}

std::string Config_Network::getConfig(const CONFIG_KEY key)
{
	const ConfigData& data = m_umapConfigData.at(key);
	return Config::getConfig(data.strKey, data.strDefault);
}

void Config_Network::setConfig(const CONFIG_KEY key, const std::string& value)
{
	const ConfigData& data = m_umapConfigData.at(key);
	Config::setConfig(data.strKey, value);
}

void Config_Network::removeConfig(const CONFIG_KEY key)
{
	const ConfigData& data = m_umapConfigData.at(key);
	Config::removeConfig(data.strKey);
}
