#include "networking.hpp"
#include "Arduino.h"
#include "config/config_network.hpp"
#include "config/config_system.hpp"


Networking* Networking::m_instance = nullptr;


Networking::Networking()
{
	m_timeLastSyncNtp = std::chrono::steady_clock::time_point();
	m_intervalSyncNtp = std::chrono::hours(24);

	m_timerWifi.setInterval(std::chrono::seconds(5));
	m_timerWifi.addCallback([this]() { this->updateWifi(); });
	m_timerWifi.start();
}

Networking::~Networking()
{

}

Networking* Networking::init()
{
	if (!m_instance)
		m_instance = new Networking();

	return m_instance;
}

void Networking::addCallbackConnect(const func_cb_connect& func)
{
	m_vecFuncConnect.push_back(func);
}

void Networking::addCallbackDisconnect(const func_cb_disconnect& func)
{
	m_vecFuncDisonnect.push_back(func);
}

void Networking::addCallbackApStart(const func_cb_ap_start& func)
{
	m_vecFuncApStart.push_back(func);
}

void Networking::addCallbackApStop(const func_cb_ap_stop& func)
{
	m_vecFuncApStop.push_back(func);
}

bool Networking::isValidIPv4(const std::string& ip)
{
	if (ip.empty())
		return false;

	std::vector<std::string> octets;
	size_t start = 0;
	size_t end = ip.find('.');

	// Split by dots
	while (end != std::string::npos)
	{
		octets.push_back(ip.substr(start, end - start));
		start = end + 1;
		end = ip.find('.', start);
	}
	octets.push_back(ip.substr(start));

	// Must have exactly 4 octets
	if (octets.size() != 4)
		return false;

	// Validate each octet
	for (const auto& octet : octets)
	{
		if (octet.empty())
			return false;

		// Check if all characters are digits
		for (char c : octet)
		{
			if (std::isdigit(c) == 0)
				return false;
		}

		// Convert to integer and check range
		int value = 0;

		for (char c : octet)
		{
			value = value * 10 + (c - '0');

			// Early exit if value exceeds 255
			if (value > 255)
				return false;
		}
	}

	return true;
}

Networking::SettingsNetwork Networking::loadSettingsNetwork()
{
	Config_Network cfgNet;

	SettingsNetwork settings;
	settings.ssid = cfgNet.getConfig(Config_Network::SSID);
	settings.password = cfgNet.getConfig(Config_Network::PASSWORD);
	settings.mode = cfgNet.getConfig(Config_Network::MODE);
	settings.hostname = cfgNet.getConfig(Config_Network::HOSTNAME);
	settings.ip = cfgNet.getConfig(Config_Network::IP);
	settings.subnet = cfgNet.getConfig(Config_Network::SUBNET);
	settings.gateway = cfgNet.getConfig(Config_Network::GATEWAY);
	settings.dns_1 = cfgNet.getConfig(Config_Network::DNS_1);
	settings.dns_2 = cfgNet.getConfig(Config_Network::DNS_2);

	return settings;
}

void Networking::saveSettingsNetwork(const SettingsNetwork& settings)
{
	Config_Network cfgNet;

	cfgNet.setConfig(Config_Network::SSID, settings.ssid);
	cfgNet.setConfig(Config_Network::PASSWORD, settings.password);
	cfgNet.setConfig(Config_Network::MODE, settings.mode);
	cfgNet.setConfig(Config_Network::HOSTNAME, settings.hostname);
	cfgNet.setConfig(Config_Network::IP, settings.ip);
	cfgNet.setConfig(Config_Network::SUBNET, settings.subnet);
	cfgNet.setConfig(Config_Network::GATEWAY, settings.gateway);
	cfgNet.setConfig(Config_Network::DNS_1, settings.dns_1);
	cfgNet.setConfig(Config_Network::DNS_2, settings.dns_2);
}

bool Networking::validateNetworkSettings(const SettingsNetwork& settings)
{
	// SSID is always required, password is optional (for open networks)
	if (settings.ssid.empty())
		return false;

	// MODE is required (DHCP or STATIC)
	if (settings.mode != "dhcp" && settings.mode != "static")
		return false;

	// For STATIC mode, IP and SUBNET are required
	if (settings.mode == "static")
	{
		if (!isValidIPv4(settings.ip))
			return false;

		if (!isValidIPv4(settings.subnet))
			return false;

		// Validate optional fields if present
		if (!settings.gateway.empty() && !isValidIPv4(settings.gateway))
			return false;

		if (!settings.dns_1.empty() && !isValidIPv4(settings.dns_1))
			return false;

		if (!settings.dns_2.empty() && !isValidIPv4(settings.dns_2))
			return false;
	}

	return true;
}

Networking::SettingsNtp Networking::loadSettingsNtp()
{
	Config_System cfg;

	SettingsNtp settings;
	settings.ntp_1 = cfg.getConfig(Config_System::NTP_1);
	settings.ntp_2 = cfg.getConfig(Config_System::NTP_2);
	settings.ntp_3 = cfg.getConfig(Config_System::NTP_3);

	errno = 0;
	settings.offsetGmt = std::strtol(cfg.getConfig(Config_System::TIMEZONE_OFFSET_GMT).c_str(), nullptr, 10);
	settings.offsetGmt = (errno == 0 ? settings.offsetGmt : 0);

	errno = 0;
	settings.offsetDst = std::strtol(cfg.getConfig(Config_System::TIMEZONE_OFFSET_DST).c_str(), nullptr, 10);
	settings.offsetDst = (errno == 0 ? settings.offsetDst : 0);

	return settings;
}

void Networking::saveSettingsNtp(const SettingsNtp& settings)
{
	Config_System cfg;

	cfg.setConfig(Config_System::NTP_1, settings.ntp_1);
	cfg.setConfig(Config_System::NTP_2, settings.ntp_2);
	cfg.setConfig(Config_System::NTP_3, settings.ntp_3);
	cfg.setConfig(Config_System::TIMEZONE_OFFSET_GMT, std::to_string(settings.offsetGmt));
	cfg.setConfig(Config_System::TIMEZONE_OFFSET_DST, std::to_string(settings.offsetDst));
}

bool Networking::validateNtpSettings(const SettingsNtp& settings)
{
	// At least one NTP server must be configured
	if (settings.ntp_1.empty() && settings.ntp_2.empty() && settings.ntp_3.empty())
		return false;

	// Validate GMT offset range (-12 to +14 hours in seconds)
	if (settings.offsetGmt < -12 * 3600 || settings.offsetGmt > 14 * 3600)
		return false;

	// Validate DST offset range (-1 to +1 hour in seconds)
	if (settings.offsetDst < -3600 || settings.offsetDst > 3600)
		return false;

	return true;
}

void Networking::reload()
{
	// FIXME WPS

	if (WiFi.getMode() == WIFI_STA)
	{
		WiFi.disconnect();

		if (m_wifiStateLast != WL_DISCONNECTED)
			onDisconnect();
	}
	else if (WiFi.getMode() == WIFI_AP)
	{
		onApStop();

		WiFi.softAPdisconnect();
	}

	m_wifiStateLast = WL_DISCONNECTED;

	const SettingsNetwork settings = loadSettingsNetwork();

	// Verify settings
	if (validateNetworkSettings(settings))
	{
		m_configured = true;

		WiFi.mode(WIFI_MODE_STA);

		// Apply settings
		if (settings.mode == "static")
		{
			WiFi.config(settings.ip.c_str(), settings.gateway.c_str(), settings.subnet.c_str(), settings.dns_1.c_str(), settings.dns_2.c_str());
		}

		WiFi.setHostname(settings.hostname.c_str());
		WiFi.begin(settings.ssid.c_str(), settings.password.c_str());

		// Force NTP sync on next connect
		m_timeLastSyncNtp = std::chrono::steady_clock::time_point();
	}
	else
	{
		// Invalid settings, start AP mode
		m_configured = false;

		WiFi.mode(WIFI_MODE_AP);
		WiFi.softAPConfig("192.168.10.1", "192.168.10.1", "255.255.255.0");
		WiFi.softAP("ESP32-SmartMeter");

		onApStart();
	}

	m_started = true;
}

void Networking::update()
{
	if (!m_started)
		reload();

	m_timerWifi.update();
}

void Networking::updateWifi()
{
	if (m_configured)
		updateWifiStation();
	else
		updateWifiAp();
}

void Networking::updateWifiStation()
{
	const wl_status_t wifiState = WiFi.status();

	if (wifiState == WL_CONNECTED)
	{
		if (wifiState != m_wifiStateLast)
			onConnect();

		checkNtp();
	}
	else
	{
		if (wifiState != m_wifiStateLast)
			onDisconnect();

		WiFi.reconnect();
	}

	m_wifiStateLast = wifiState;
}

void Networking::updateWifiAp()
{

}

void Networking::checkNtp()
{
	const std::chrono::steady_clock::time_point timeNow = std::chrono::steady_clock::now();

	if (timeNow - m_timeLastSyncNtp > m_intervalSyncNtp || m_timeLastSyncNtp == std::chrono::steady_clock::time_point())
		syncNtp();
}

void Networking::syncNtp()
{
	const SettingsNtp settings = loadSettingsNtp();

	configTime(settings.offsetGmt, settings.offsetDst, settings.ntp_1.c_str(), settings.ntp_2.c_str(), settings.ntp_3.c_str());

	struct tm timeinfo;

	if (getLocalTime(&timeinfo, 500))
		m_timeLastSyncNtp = std::chrono::steady_clock::now();
}

void Networking::onConnect()
{
	log_i("--- WiFi connected. IP: %s | Hostname: %s | MAC: %s", WiFi.localIP().toString().c_str(), WiFi.getHostname(), WiFi.macAddress().c_str());

	for (const auto& func : m_vecFuncConnect)
	{
		func();
	}
}

void Networking::onDisconnect()
{
	log_i("--- WiFi disconnected.");

	for (const auto& func : m_vecFuncDisonnect)
	{
		func();
	}
}

void Networking::onApStart()
{
	log_i("--- Access Point started.");

	for (const auto& func : m_vecFuncApStart)
	{
		func();
	}
}

void Networking::onApStop()
{
	log_i("--- Access Point stopped.");

	for (const auto& func : m_vecFuncApStop)
	{
		func();
	}
}
