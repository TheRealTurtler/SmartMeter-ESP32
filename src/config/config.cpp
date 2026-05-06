#include "config.hpp"

#ifdef ESP32
#include <nvs_flash.h>
#endif


Config::Config(const std::string& ns):
	m_namespace(ns)
{
	m_pref.begin(m_namespace.c_str());
}

Config::~Config()
{
	m_pref.end();
}

std::string Config::getConfig(const std::string& key, const std::string& valueDefault)
{
	if (m_pref.isKey(key.c_str()))
		return m_pref.getString(key.c_str()).c_str();

	return valueDefault;
}

void Config::setConfig(const std::string& key, const std::string& value)
{
	if (value == "")
	{
		if (m_pref.isKey(key.c_str()))
			removeConfig(key);
	}
	else if (getConfig(key.c_str(), "") != value)
		m_pref.putString(key.c_str(), value.c_str());
}

void Config::removeConfig(const std::string& key)
{
	if (m_pref.isKey(key.c_str()))
		m_pref.remove(key.c_str());
}

void Config::resetConfig()
{
	m_pref.clear();
}

void Config::resetConfigAll()
{
#ifdef ESP32
	nvs_flash_erase();
	nvs_flash_init();
#endif
}
