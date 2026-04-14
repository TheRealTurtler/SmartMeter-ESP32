#pragma once

#include <string>
#include <map>
#include "datapoints.hpp"
#include "measuredvalue.hpp"
#include "components/timer.hpp"
#include "components/datetime.hpp"
#include <functional>


struct DataSmartMeter
{
	unsigned long tsLastUpdate = 0;
	std::map<DATA_POINT_SMARTMETER, MeasuredValue> mapData;
};

struct DataSystem
{
	std::map<DATA_POINT_SYSTEM, MeasuredValue> mapData;
};

class DataCollector
{
public:
	enum AVG_INTERVAL
	{
		AVG_1_MIN = 1,
		AVG_5_MIN = 5,
		AVG_15_MIN = 15,
	};

	DataCollector(AVG_INTERVAL avgInterval);
	~DataCollector() = default;

	void init();
	void start();
	void update();

	void updateDatapoint(const DATA_POINT_SMARTMETER dp, const double& value, bool calcAverage = true);
	void updateDatapoint(const DATA_POINT_SYSTEM dp, const double& value, bool calcAverage = true);
	void calcDerivedValues();

	const DataSmartMeter& getDataSmartMeter() const { return m_dataSmartMeter; }
	const DataSystem& getDataSystem() const { return m_dataSystem; }

	void setTsLastUpdateSmartmeter(const unsigned long& ts) { m_dataSmartMeter.tsLastUpdate = ts; }

	typedef std::function<void(const DateTime&, const DataSmartMeter&)> func_cb_smartmeter;
	typedef std::function<void(const DateTime&, const DataSystem&)> func_cb_system;

	void setCallbackSmartmeter(func_cb_smartmeter cb) { m_cbSmartmeter = cb; }
	void setCallbackSystem(func_cb_system cb) { m_cbSystem = cb; }

private:
	const int64_t m_avgInterval_us;

	DataSmartMeter m_dataSmartMeter;
	DataSystem m_dataSystem;

	Timer m_timerAverage;
	DateTime m_dtLastAverage;

	func_cb_smartmeter m_cbSmartmeter;
	func_cb_system m_cbSystem;

	void callbackAverage();
	void resetAverage();

	DateTime roundDateTime(const DateTime& dt) const;

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
