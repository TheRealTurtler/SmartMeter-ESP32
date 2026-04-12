#include "httpserver.hpp"

#include <iomanip>
#include <sstream>
#include <ctime>

#include "html.hpp"


HttpServer::HttpServer(const DataCollector& dc, uint16_t port):
	m_server(WebServer(port)),
	m_api(dc, &m_server),
	m_dc(dc)
{

}

void HttpServer::init()
{
	m_server.on("/", [this]() { this->getHome(); });
	m_server.on("/ping", [this]() { this->getPing(); });

	m_api.init();
}

void HttpServer::start()
{
	m_server.begin();
}

void HttpServer::stop()
{
	m_server.stop();
}

void HttpServer::update()
{
	m_server.handleClient();
}

void HttpServer::getHome()
{
	Serial.println("HTTP GET HOME");

	/*
	// Check query parameter for language (default: de)
	std::string lang = "de";
	if (m_server.hasArg("lang"))
	{
		lang = m_server.arg("lang").c_str();
	}

	// Select template based on language
	const std::string& template_html = (lang == "en") ? HtmlTemplates::HTML_TEMPLATE_EN : HtmlTemplates::HTML_TEMPLATE_DE;

	// Fill template with data
	std::string html = fillTemplate(template_html);
	*/

	m_server.send(200, "text/html; charset=utf-8", Html::index.c_str());
}

void HttpServer::getPing()
{
	Serial.println("HTTP GET PING");
	m_server.send(200, "text/plain", "Pong");
}

std::string HttpServer::fillTemplate(const std::string& template_html)
{
	std::string result = template_html;
	auto& dataPoints = m_dc.getDataPoints();

	/*
	// Helper lambda to replace placeholder
	auto replacePlaceholder = [&result](const std::string& placeholder, const std::string& value)
		{
			size_t pos = 0;
			const std::string search = "{{" + placeholder + "}}";
			while ((pos = result.find(search, pos)) != std::string::npos)
			{
				result.replace(pos, search.length(), value);
				pos += value.length();
			}
		};

	// Add timestamp
	replacePlaceholder("TIMESTAMP", getCurrentTimestamp());

	// Map DATA_POINT enum to template keys
	struct DataPointMapping
	{
		DATA_POINT dp;
		const char* keyNow;
		const char* keyAvg;
	};

	const DataPointMapping mappings[] = {
		// Power
		{DP_ACTIVE_POWER_TOTAL, "ACTIVE_POWER_TOTAL", "ACTIVE_POWER_TOTAL_AVG"},
		{DP_ACTIVE_POWER_L1, "ACTIVE_POWER_L1", "ACTIVE_POWER_L1_AVG"},
		{DP_ACTIVE_POWER_L2, "ACTIVE_POWER_L2", "ACTIVE_POWER_L2_AVG"},
		{DP_ACTIVE_POWER_L3, "ACTIVE_POWER_L3", "ACTIVE_POWER_L3_AVG"},
		{DP_REACTIVE_POWER_TOTAL, "REACTIVE_POWER_TOTAL", "REACTIVE_POWER_TOTAL_AVG"},
		{DP_REACTIVE_POWER_L1, "REACTIVE_POWER_L1", "REACTIVE_POWER_L1_AVG"},
		{DP_REACTIVE_POWER_L2, "REACTIVE_POWER_L2", "REACTIVE_POWER_L2_AVG"},
		{DP_REACTIVE_POWER_L3, "REACTIVE_POWER_L3", "REACTIVE_POWER_L3_AVG"},
		{DP_APPARENT_POWER_TOTAL, "APPARENT_POWER_TOTAL", "APPARENT_POWER_TOTAL_AVG"},
		{DP_APPARENT_POWER_L1, "APPARENT_POWER_L1", "APPARENT_POWER_L1_AVG"},
		{DP_APPARENT_POWER_L2, "APPARENT_POWER_L2", "APPARENT_POWER_L2_AVG"},
		{DP_APPARENT_POWER_L3, "APPARENT_POWER_L3", "APPARENT_POWER_L3_AVG"},

		// Energy
		{DP_ACTIVE_ENERGY_IMPORT, "ACTIVE_ENERGY_IMPORT", "ACTIVE_ENERGY_IMPORT_AVG"},
		{DP_ACTIVE_ENERGY_EXPORT, "ACTIVE_ENERGY_EXPORT", "ACTIVE_ENERGY_EXPORT_AVG"},

		// Voltage
		{DP_VOLTAGE_L1N, "VOLTAGE_L1N", "VOLTAGE_L1N_AVG"},
		{DP_VOLTAGE_L2N, "VOLTAGE_L2N", "VOLTAGE_L2N_AVG"},
		{DP_VOLTAGE_L3N, "VOLTAGE_L3N", "VOLTAGE_L3N_AVG"},
		{DP_VOLTAGE_L1L2, "VOLTAGE_L1L2", "VOLTAGE_L1L2_AVG"},
		{DP_VOLTAGE_L2L3, "VOLTAGE_L2L3", "VOLTAGE_L2L3_AVG"},
		{DP_VOLTAGE_L3L1, "VOLTAGE_L3L1", "VOLTAGE_L3L1_AVG"},

		// Current
		{DP_CURRENT_L1, "CURRENT_L1", "CURRENT_L1_AVG"},
		{DP_CURRENT_L2, "CURRENT_L2", "CURRENT_L2_AVG"},
		{DP_CURRENT_L3, "CURRENT_L3", "CURRENT_L3_AVG"},

		// Power Factor
		{DP_POWER_FACTOR_TOTAL, "POWER_FACTOR_TOTAL", "POWER_FACTOR_TOTAL_AVG"},
		{DP_POWER_FACTOR_L1, "POWER_FACTOR_L1", "POWER_FACTOR_L1_AVG"},
		{DP_POWER_FACTOR_L2, "POWER_FACTOR_L2", "POWER_FACTOR_L2_AVG"},
		{DP_POWER_FACTOR_L3, "POWER_FACTOR_L3", "POWER_FACTOR_L3_AVG"},

		// Angles
		{DP_ANGLE_VOLTAGE_L1L2, "ANGLE_VOLTAGE_L1L2", "ANGLE_VOLTAGE_L1L2_AVG"},
		{DP_ANGLE_VOLTAGE_L1L3, "ANGLE_VOLTAGE_L1L3", "ANGLE_VOLTAGE_L1L3_AVG"},
		{DP_ANGLE_CURRENT_L1, "ANGLE_CURRENT_L1", "ANGLE_CURRENT_L1_AVG"},
		{DP_ANGLE_CURRENT_L2, "ANGLE_CURRENT_L2", "ANGLE_CURRENT_L2_AVG"},
		{DP_ANGLE_CURRENT_L3, "ANGLE_CURRENT_L3", "ANGLE_CURRENT_L3_AVG"},

		// System
		{DP_FREQUENCY, "FREQUENCY", "FREQUENCY_AVG"},
		{DP_MCU_USAGE, "MCU_USAGE", "MCU_USAGE_AVG"},
		{DP_WIFI_RSSI, "WIFI_RSSI", "WIFI_RSSI_AVG"},
		{DP_TEMPERATURE, "TEMPERATURE", "TEMPERATURE_AVG"},
	};

	// Fill all data points
	for (const auto& mapping : mappings)
	{
		auto it = dataPoints.find(mapping.dp);
		if (it != dataPoints.end())
		{
			const MeasuredValue& mv = it->second;
			replacePlaceholder(mapping.keyNow, formatValue(mv.getValueNow()));
			replacePlaceholder(mapping.keyAvg, formatValue(mv.getValueAverage()));
		}
	}
	*/

	return result;
}

std::string HttpServer::formatValue(double value, uint8_t decimals)
{
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(decimals) << value;
	return oss.str();
}

std::string HttpServer::getCurrentTimestamp()
{
	time_t now = time(nullptr);
	struct tm* timeinfo = localtime(&now);

	char buffer[100];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

	return std::string(buffer);
}
