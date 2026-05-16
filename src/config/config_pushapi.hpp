#pragma once

#include "config.hpp"


class Config_PushAPI : public Config
{
public:
	Config_PushAPI();
	~Config_PushAPI() = default;

	enum CONFIG_KEY
	{
		ENABLE,
		SERVER_HOST,
		SERVER_PATH_SMARTMETER,
		SERVER_PATH_SYSTEM,
		BATCH_SIZE,
		DISABLE_WIFI,
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
