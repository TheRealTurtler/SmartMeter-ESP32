#pragma once

#include <string>
#include <map>
#include "datapoints.hpp"
#include "measuredvalue.hpp"

class DataCollector
{
public:
	DataCollector() = default;
	~DataCollector() = default;

	void update();

	void updateDatapoint(const DATA_POINT_SMARTMETER dp, const double& value, bool calcAverage = true);
	void updateDatapoint(const DATA_POINT_SYSTEM dp, const double& value, bool calcAverage = true);
	void calcDerivedValues();

	const int64_t& getTsLastUpdateSmartmeter() const { return m_tsLastUpdateSmartmeter; }
	void setTsLastUpdateSmartmeter(const int64_t& ts) { m_tsLastUpdateSmartmeter = ts; }

	void resetAverages();

	const std::map<DATA_POINT_SMARTMETER, MeasuredValue>& getDataPointsSmartmeter() const { return m_mapDataPointsSmartmeter; }
	const std::map<DATA_POINT_SYSTEM, MeasuredValue>& getDataPointsSystem() const { return m_mapDataPointsSystem; }

private:
	std::map<DATA_POINT_SMARTMETER, MeasuredValue> m_mapDataPointsSmartmeter;
	std::map<DATA_POINT_SYSTEM, MeasuredValue> m_mapDataPointsSystem;

	int64_t m_tsLastUpdateSmartmeter = -1;

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
