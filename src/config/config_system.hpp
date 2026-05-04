#pragma once

#include "config.hpp"


class Config_System : public Config
{
public:
	Config_System();
	~Config_System() = default;

	enum CONFIG_KEY
	{
		NTP_1,
		NTP_2,
		NTP_3,
		TIMEZONE_OFFSET_GMT,
		TIMEZONE_OFFSET_DST,
	};

	std::string getConfig(const CONFIG_KEY key);
	void setConfig(const CONFIG_KEY key, const std::string& value);
	void removeConfig(const CONFIG_KEY key);

private:
	struct ConfigData
	{
		const std::string strKey;
		const std::string strDefault;
	};

	static const std::unordered_map<CONFIG_KEY, ConfigData> m_umapConfigData;
};
