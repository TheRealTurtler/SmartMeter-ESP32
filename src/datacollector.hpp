#pragma once

#include <string>
#include <map>
#include "measuredvalue.hpp"

enum DATA_POINT
{
	// Smartmeter
	DP_ACTIVE_POWER_TOTAL,
	DP_ACTIVE_POWER_L1,
	DP_ACTIVE_POWER_L2,
	DP_ACTIVE_POWER_L3,

	DP_REACTIVE_POWER_TOTAL,
	DP_REACTIVE_POWER_L1,
	DP_REACTIVE_POWER_L2,
	DP_REACTIVE_POWER_L3,

	DP_APPARENT_POWER_TOTAL,
	DP_APPARENT_POWER_L1,
	DP_APPARENT_POWER_L2,
	DP_APPARENT_POWER_L3,

	DP_ACTIVE_ENERGY_IMPORT,
	DP_ACTIVE_ENERGY_EXPORT,

	DP_VOLTAGE_L1N,
	DP_VOLTAGE_L2N,
	DP_VOLTAGE_L3N,

	DP_VOLTAGE_L1L2,
	DP_VOLTAGE_L2L3,
	DP_VOLTAGE_L3L1,

	DP_CURRENT_L1,
	DP_CURRENT_L2,
	DP_CURRENT_L3,

	DP_POWER_FACTOR_TOTAL,
	DP_POWER_FACTOR_L1,
	DP_POWER_FACTOR_L2,
	DP_POWER_FACTOR_L3,

	DP_ANGLE_VOLTAGE_L1L2,
	DP_ANGLE_VOLTAGE_L1L3,

	DP_ANGLE_CURRENT_L1,
	DP_ANGLE_CURRENT_L2,
	DP_ANGLE_CURRENT_L3,

	DP_FREQUENCY,

	// General
	DP_MCU_USAGE,
	DP_WIFI_RSSI,
	DP_TEMPERATURE,
};

class DataCollector
{
public:
	DataCollector() = default;
	~DataCollector() = default;

	void update();

	void updateDatapoint(const DATA_POINT dp, const double& value);
	void calcDerivedValues();

	void resetAverages();

	const std::map<DATA_POINT, MeasuredValue>& getDataPoints() const { return m_mapDataPoints; }
	MeasuredValue getDataPoint(const DATA_POINT dp);

	// FIXME Move to HttpAPI
	std::string getJsonStatus() const;

private:
	std::map<DATA_POINT, MeasuredValue> m_mapDataPoints;

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
