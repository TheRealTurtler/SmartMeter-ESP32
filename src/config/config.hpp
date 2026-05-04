#pragma once

#include <string>
#include <Preferences.h>


class Config
{
public:
	Config(const std::string& ns);
	virtual ~Config();

	void resetConfig();
	static void resetConfigAll();

protected:
	std::string getConfig(const std::string& key, const std::string& valueDefault);
	void setConfig(const std::string& key, const std::string& value);
	void removeConfig(const std::string& key);

private:
	const std::string m_namespace;

	Preferences m_pref;
};
