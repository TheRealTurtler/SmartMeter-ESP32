#include "config_pushapi.hpp"


const std::unordered_map<Config_PushAPI::CONFIG_KEY, Config_PushAPI::ConfigData> Config_PushAPI::m_umapConfigData =
{
	{ ENABLE,						{"enable",			"false"} },
	{ SERVER_HOST,					{"host",			""} },
	{ SERVER_PATH_SMARTMETER,		{"path_meter",		""} },
	{ SERVER_PATH_SYSTEM,			{"path_system",		""} },
	{ BATCH_SIZE,					{"batch_size",		"1"} },
	{ DISABLE_WIFI,					{"disable_wifi",	"false"} },
};


Config_PushAPI::Config_PushAPI():
	Config("push_api")
{

}

std::string Config_PushAPI::getConfig(const CONFIG_KEY key)
{
	const ConfigData& data = m_umapConfigData.at(key);
	return Config::getConfig(data.strKey, data.strDefault);
}

void Config_PushAPI::setConfig(const CONFIG_KEY key, const std::string& value)
{
	const ConfigData& data = m_umapConfigData.at(key);
	Config::setConfig(data.strKey, value);
}

void Config_PushAPI::removeConfig(const CONFIG_KEY key)
{
	const ConfigData& data = m_umapConfigData.at(key);
	Config::removeConfig(data.strKey);
}
