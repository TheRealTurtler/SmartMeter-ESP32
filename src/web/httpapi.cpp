#include "httpapi.hpp"
#include "networking.hpp"
#include "web/client/httpclient.hpp"
#include "config/config.hpp"


HttpAPI::HttpAPI()
{

}

void HttpAPI::init()
{

}

void HttpAPI::update()
{

}

ArduinoJson::JsonDocument HttpAPI::buildJsonStatus() const
{
	ArduinoJson::JsonDocument doc;
	doc["status"] = "ok";

	return doc;
}

ArduinoJson::JsonDocument HttpAPI::buildJsonSmartmeter(const std::chrono::system_clock::time_point& tp, const DataSmartMeter& data, const std::vector<std::string>& vecFilter) const
{
	ArduinoJson::JsonDocument doc;

	if (data.timeLastUpdate != std::chrono::steady_clock::time_point())
	{
		const auto millisDataset = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());

		doc["status"] = "ok";
		doc["time_dataset"] = millisDataset.count();

		const auto timeNowSteady = std::chrono::steady_clock::now();
		const auto timeNowSystem = std::chrono::system_clock::now();

		const auto timeDiff = (timeNowSteady - data.timeLastUpdate);
		const auto timeLastUpdate = (timeNowSystem - timeDiff);

		const auto millisLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(timeLastUpdate.time_since_epoch());

		doc["time_last_update"] = millisLastUpdate.count();

		for (const auto& [dp, mv] : data.mapData)
		{
			if (gc_mapDataPointInfoSmartmeter.contains(dp))
			{
				const std::string& key = gc_mapDataPointInfoSmartmeter.at(dp);

				// If filter is set, then only include datapoints that are in the filter list
				if (!vecFilter.empty() && std::find(vecFilter.begin(), vecFilter.end(), key) == vecFilter.end())
					continue;

				ArduinoJson::JsonObject obj = doc[key].to<JsonObject>();

				if (dp == DP_ACTIVE_ENERGY_IMPORT || dp == DP_ACTIVE_ENERGY_EXPORT)
					obj["abs"] = mv.getValueNow();
				else
				{
					obj["now"] = mv.getValueNow();
					obj["avg"] = mv.getValueAverage();
				}
			}
		}
	}
	else
	{
		doc["status"] = "no_data";
	}

	return doc;
}

ArduinoJson::JsonDocument HttpAPI::buildJsonSystem(const std::chrono::system_clock::time_point& tp, const DataSystem& data, const std::vector<std::string>& vecFilter) const
{
	ArduinoJson::JsonDocument doc;

	const auto timeNowSystem = std::chrono::system_clock::now();
	const auto millisNow = std::chrono::duration_cast<std::chrono::milliseconds>(timeNowSystem.time_since_epoch());
	const auto millisDataset = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());

	doc["status"] = "ok";
	doc["time_dataset"] = millisDataset.count();
	doc["time_now"] = millisNow.count();

	for (const auto& [dp, mv] : data.mapData)
	{
		if (gc_mapDataPointInfoSystem.contains(dp))
		{
			const std::string& key = gc_mapDataPointInfoSystem.at(dp);

			// If filter is set, then only include datapoints that are in the filter list
			if (!vecFilter.empty() && std::find(vecFilter.begin(), vecFilter.end(), key) == vecFilter.end())
				continue;

			ArduinoJson::JsonObject obj = doc[key].to<JsonObject>();
			obj["now"] = mv.getValueNow();
			obj["avg"] = mv.getValueAverage();
		}
	}

	return doc;
}

ArduinoJson::JsonDocument HttpAPI::buildJsonSettings(const std::vector<std::string>& vecFilter) const
{
	ArduinoJson::JsonDocument doc;
	doc["status"] = "ok";

	{
		const Networking::SettingsNetwork settings = Networking::loadSettingsNetwork();

		ArduinoJson::JsonObject obj = doc["network"].to<JsonObject>();
		obj["ssid"] = settings.ssid;
		obj["password"] = settings.password;
		obj["mode"] = settings.mode;
		obj["hostname"] = settings.hostname;
		obj["ip"] = settings.ip;
		obj["subnet"] = settings.subnet;
		obj["gateway"] = settings.gateway;
		obj["dns_1"] = settings.dns_1;
		obj["dns_2"] = settings.dns_2;
	}

	{
		const Networking::SettingsNtp settings = Networking::loadSettingsNtp();

		ArduinoJson::JsonObject obj = doc["system"].to<JsonObject>();
		obj["ntp_1"] = settings.ntp_1;
		obj["ntp_2"] = settings.ntp_2;
		obj["ntp_3"] = settings.ntp_3;
		obj["timezone_offset_gmt"] = settings.offsetGmt;
		obj["timezone_offset_dst"] = settings.offsetDst;
	}

	{
		HttpClient::Settings settings = HttpClient::loadSettings();

		ArduinoJson::JsonObject obj = doc["push_api"].to<JsonObject>();
		obj["enable"] = settings.enable;
		obj["server_host"] = settings.serverHost;
		obj["server_path_smartmeter"] = settings.serverLocationSmartMeter;
		obj["server_path_system"] = settings.serverLocationSystem;
	}

	return doc;
}

ArduinoJson::JsonDocument HttpAPI::decodeJsonSettings(const ArduinoJson::JsonDocument& docReceived) const
{
	ArduinoJson::JsonDocument docResponse;

	if (docReceived["factory_reset"] == true)
	{
		Config::resetConfigAll();

		docResponse["status"] = "ok";
		return docResponse;
	}

	const ArduinoJson::JsonObjectConst objNetwork = docReceived["network"];
	const ArduinoJson::JsonObjectConst objSystem = docReceived["system"];
	const ArduinoJson::JsonObjectConst objPushApi = docReceived["push_api"];

	Networking::SettingsNetwork settingsNetwork;
	settingsNetwork.ssid = objNetwork["ssid"].as<std::string>();
	settingsNetwork.password = objNetwork["password"].as<std::string>();
	settingsNetwork.mode = objNetwork["mode"].as<std::string>();
	settingsNetwork.hostname = objNetwork["hostname"].as<std::string>();
	settingsNetwork.ip = objNetwork["ip"].as<std::string>();
	settingsNetwork.subnet = objNetwork["subnet"].as<std::string>();
	settingsNetwork.gateway = objNetwork["gateway"].as<std::string>();
	settingsNetwork.dns_1 = objNetwork["dns_1"].as<std::string>();
	settingsNetwork.dns_2 = objNetwork["dns_2"].as<std::string>();

	Networking::SettingsNtp settingsNtp;
	settingsNtp.ntp_1 = objSystem["ntp_1"].as<std::string>();
	settingsNtp.ntp_2 = objSystem["ntp_2"].as<std::string>();
	settingsNtp.ntp_3 = objSystem["ntp_3"].as<std::string>();
	settingsNtp.offsetGmt = objSystem["timezone_offset_gmt"].as<long>();
	settingsNtp.offsetDst = objSystem["timezone_offset_dst"].as<long>();

	HttpClient::Settings settingsPushApi;
	settingsPushApi.enable = objPushApi["enable"].as<bool>();
	settingsPushApi.serverHost = objPushApi["server_host"].as<std::string>();
	settingsPushApi.serverLocationSmartMeter = objPushApi["server_path_smartmeter"].as<std::string>();
	settingsPushApi.serverLocationSystem = objPushApi["server_path_system"].as<std::string>();

	const bool okNetwork = Networking::validateNetworkSettings(settingsNetwork);
	const bool okSystem = Networking::validateNtpSettings(settingsNtp);
	const bool okPushApi = HttpClient::validateSettings(settingsPushApi);

	docResponse["network"]["status"] = (okNetwork) ? "ok" : "invalid";
	docResponse["system"]["status"] = (okSystem) ? "ok" : "invalid";
	docResponse["push_api"]["status"] = (okPushApi) ? "ok" : "invalid";

	Serial.println("Validation results:");
	Serial.print(" - Network: ");
	Serial.println(okNetwork ? "ok" : "invalid");
	Serial.print(" - System: ");
	Serial.println(okSystem ? "ok" : "invalid");
	Serial.print(" - Push API: ");
	Serial.println(okPushApi ? "ok" : "invalid");

	if (okNetwork && okSystem && okPushApi)
	{
		docResponse["status"] = "ok";

		Networking::saveSettingsNetwork(settingsNetwork);
		Networking::saveSettingsNtp(settingsNtp);
		HttpClient::saveSettings(settingsPushApi);
	}
	else
		docResponse["status"] = "invalid";

	Serial.println("Response JSON:");
	{
		std::string test;
		ArduinoJson::serializeJson(docResponse, test);
		Serial.println(test.c_str());
	}

	return docResponse;
}
