#pragma once

#include <string>
#include <map>
#include "datapoints.hpp"
#include "measuredvalue.hpp"
#include "components/timer.hpp"
#include <functional>
#include <chrono>


struct DataSmartMeter
{
	std::chrono::steady_clock::time_point timeLastUpdate;
	std::map<DATA_POINT_SMARTMETER, MeasuredValue> mapData;
};

struct DataSystem
{
	std::map<DATA_POINT_SYSTEM, MeasuredValue> mapData;
};

class DataCollector
{
public:
	DataCollector(const std::chrono::milliseconds& avgInterval);
	~DataCollector() = default;

	void init();
	void start();
	void update();

	void updateDatapoint(const DATA_POINT_SMARTMETER dp, const double& value, bool calcAverage = true);
	void updateDatapoint(const DATA_POINT_SYSTEM dp, const double& value, bool calcAverage = true);
	void calcDerivedValues();

	const DataSmartMeter& getDataSmartMeter() const { return m_dataSmartMeter; }
	const DataSystem& getDataSystem() const { return m_dataSystem; }

	void setTimeLastUpdateSmartmeter(const std::chrono::steady_clock::time_point& timePoint) { m_dataSmartMeter.timeLastUpdate = timePoint; }

	typedef std::function<void(const std::chrono::system_clock::time_point&, const DataSmartMeter&)> func_cb_smartmeter;
	typedef std::function<void(const std::chrono::system_clock::time_point&, const DataSystem&)> func_cb_system;

	void setCallbackSmartmeter(func_cb_smartmeter cb) { m_cbSmartmeter = cb; }
	void setCallbackSystem(func_cb_system cb) { m_cbSystem = cb; }

private:
	const std::chrono::milliseconds m_avgInterval;

	DataSmartMeter m_dataSmartMeter;
	DataSystem m_dataSystem;

	Timer m_timerAverage;
	std::chrono::system_clock::time_point m_timeLastAverage;

	func_cb_smartmeter m_cbSmartmeter;
	func_cb_system m_cbSystem;

	void callbackAverage();
	void resetAverage();

	std::chrono::system_clock::time_point roundTimePoint(const std::chrono::system_clock::time_point& tp) const;

	void calcDerivedVoltage();
	void calcDerivedPowerFactor();
	void calcDerivedReactivePower();
	void calcDerivedApparentPower();

	double calcVoltageLL(const double& valueLN1, const double& valueLN2, const double& angleUU);
	double calcPowerFactor(const double& angleUI);
	double calcPowerFactor(const double& activePower, const double& apparentPower);
	double calcReactivePower(const double& activePower, const double& powerFactor);
	double calcApparentPower(const double& activePower, const double& reactivePower);
};
