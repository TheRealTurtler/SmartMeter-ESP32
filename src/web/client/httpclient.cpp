#include "httpclient.hpp"


HttpClient::HttpClient(const HttpAPI& api, DataCollector& dc):
	m_api(api)
{
	auto cbSmartmeter = [this](const DateTime& dt, const DataSmartMeter& data) { this->callbackSmartmeter(dt, data); };
	auto cbSystem = [this](const DateTime& dt, const DataSystem& data) { this->callbackSystem(dt, data); };

	dc.setCallbackSmartmeter(cbSmartmeter);
	dc.setCallbackSystem(cbSystem);
}

void HttpClient::init()
{

}

void HttpClient::update()
{

}

void HttpClient::callbackSmartmeter(const DateTime& dt, const DataSmartMeter& data)
{

}

void HttpClient::callbackSystem(const DateTime& dt, const DataSystem& data)
{

}
