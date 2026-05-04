#include "config_system.hpp"


const std::unordered_map<Config_System::CONFIG_KEY, Config_System::ConfigData> Config_System::m_umapConfigData =
{
	{ NTP_1,					{"ntp_1",		"pool.ntp.org"} },
	{ NTP_2,					{"ntp_2",		"time.nist.gov"} },
	{ NTP_3,					{"ntp_3",		"time.google.com"} },
	{ TIMEZONE_OFFSET_GMT,		{"tz_off_gmt",	"0"} },
	{ TIMEZONE_OFFSET_DST,		{"tz_off_dst",	"0"} },
};


Config_System::Config_System():
	Config("system")
{

}

std::string Config_System::getConfig(const CONFIG_KEY key)
{
	const ConfigData& data = m_umapConfigData.at(key);
	return Config::getConfig(data.strKey, data.strDefault);
}

void Config_System::setConfig(const CONFIG_KEY key, const std::string& value)
{
	const ConfigData& data = m_umapConfigData.at(key);
	Config::setConfig(data.strKey, value);
}

void Config_System::removeConfig(const CONFIG_KEY key)
{
	const ConfigData& data = m_umapConfigData.at(key);
	Config::removeConfig(data.strKey);
}
