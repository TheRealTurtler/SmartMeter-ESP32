#pragma once

#include "config.hpp"
#include <unordered_map>

class Config_Network : public Config
{
public:
	Config_Network();
	~Config_Network() = default;

	enum CONFIG_KEY
	{
		SSID,
		PASSWORD,
		MODE,
		HOSTNAME,
		IP,
		SUBNET,
		GATEWAY,
		DNS_1,
		DNS_2,
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
