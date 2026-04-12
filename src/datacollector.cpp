#include "datacollector.hpp"
#include <cmath>
#include <algorithm>

// FIXME remove
#include <Arduino.h>


void DataCollector::update()
{
	static unsigned int ctr = 0;

	if (++ctr % 1000 == 0)
	{
		ctr = 0;

		Serial.print("Samples: ");
		Serial.println(m_mapDataPoints[DP_ACTIVE_POWER_TOTAL].getSampleCount());

		Serial.print("MCU Usage: ");
		Serial.println(m_mapDataPoints[DP_MCU_USAGE].getValueAverage());
		Serial.print("WiFi RSSI: ");
		Serial.println(m_mapDataPoints[DP_WIFI_RSSI].getValueAverage());

		Serial.print("Voltage L1-N: ");
		Serial.print(m_mapDataPoints[DP_VOLTAGE_L1N].getValueNow());
		Serial.print(" L2-N: ");
		Serial.print(m_mapDataPoints[DP_VOLTAGE_L2N].getValueNow());
		Serial.print(" L3-N: ");
		Serial.println(m_mapDataPoints[DP_VOLTAGE_L3N].getValueNow());

		Serial.print("Voltage L1-L2: ");
		Serial.print(m_mapDataPoints[DP_VOLTAGE_L1L2].getValueNow());
		Serial.print(" L2-L3: ");
		Serial.print(m_mapDataPoints[DP_VOLTAGE_L2L3].getValueNow());
		Serial.print(" L3-L1: ");
		Serial.println(m_mapDataPoints[DP_VOLTAGE_L3L1].getValueNow());

		Serial.print("Active Power L1: ");
		Serial.print(m_mapDataPoints[DP_ACTIVE_POWER_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_mapDataPoints[DP_ACTIVE_POWER_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.print(m_mapDataPoints[DP_ACTIVE_POWER_L3].getValueNow());
		Serial.print(" Total: ");
		Serial.println(m_mapDataPoints[DP_ACTIVE_POWER_TOTAL].getValueNow());

		Serial.print("Reactive Power L1: ");
		Serial.print(m_mapDataPoints[DP_REACTIVE_POWER_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_mapDataPoints[DP_REACTIVE_POWER_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.print(m_mapDataPoints[DP_REACTIVE_POWER_L3].getValueNow());
		Serial.print(" Total: ");
		Serial.println(m_mapDataPoints[DP_REACTIVE_POWER_TOTAL].getValueNow());

		Serial.print("Apparent Power L1: ");
		Serial.print(m_mapDataPoints[DP_APPARENT_POWER_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_mapDataPoints[DP_APPARENT_POWER_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.print(m_mapDataPoints[DP_APPARENT_POWER_L3].getValueNow());
		Serial.print(" Total: ");
		Serial.println(m_mapDataPoints[DP_APPARENT_POWER_TOTAL].getValueNow());

		Serial.print("Power Factor L1: ");
		Serial.print(m_mapDataPoints[DP_POWER_FACTOR_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_mapDataPoints[DP_POWER_FACTOR_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.print(m_mapDataPoints[DP_POWER_FACTOR_L3].getValueNow());
		Serial.print(" Total: ");
		Serial.println(m_mapDataPoints[DP_POWER_FACTOR_TOTAL].getValueNow());

		Serial.print("Current L1: ");
		Serial.print(m_mapDataPoints[DP_CURRENT_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_mapDataPoints[DP_CURRENT_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.println(m_mapDataPoints[DP_CURRENT_L3].getValueNow());

		Serial.print("Frequency: ");
		Serial.println(m_mapDataPoints[DP_FREQUENCY].getValueNow());

		Serial.print("Angle U L1-L2: ");
		Serial.print(m_mapDataPoints[DP_ANGLE_VOLTAGE_L1L2].getValueNow());
		Serial.print(" L1-L3: ");
		Serial.println(m_mapDataPoints[DP_ANGLE_VOLTAGE_L1L3].getValueNow());

		Serial.print("Angle I L1: ");
		Serial.print(m_mapDataPoints[DP_ANGLE_CURRENT_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_mapDataPoints[DP_ANGLE_CURRENT_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.println(m_mapDataPoints[DP_ANGLE_CURRENT_L3].getValueNow());

		Serial.print("Energy Import: ");
		Serial.print(m_mapDataPoints[DP_ACTIVE_ENERGY_IMPORT].getValueNow());
		Serial.print(" Export: ");
		Serial.println(m_mapDataPoints[DP_ACTIVE_ENERGY_EXPORT].getValueNow());
	}
}

void DataCollector::updateDatapoint(const DATA_POINT dp, const double& value)
{
	m_mapDataPoints[dp].updateValue(value);
}

void DataCollector::calcDerivedValues()
{
	calcDerivedVoltage();
	calcDerivedPowerFactor();
	calcDerivedReactivePower();
	calcDerivedApparentPower();
}

void DataCollector::resetAverages()
{
	for (auto& [dp, value] : m_mapDataPoints)
	{
		value.resetAverage();
	}
}

MeasuredValue DataCollector::getDataPoint(const DATA_POINT dp)
{
	if (m_mapDataPoints.contains(dp))
		return m_mapDataPoints[dp];
	else
		return MeasuredValue();
}

std::string DataCollector::getJsonStatus() const
{
	std::string result;

	// TODO: Status of Board
	// -> In-/Outputs
	// -> WiFi RSSI
	// -> etc.

	result = "{\"STATUS\":\"OK\"}";

	return result;
}

void DataCollector::calcDerivedVoltage()
{
	// Calculate L-L voltages from L-N voltages and phase angles
	const MeasuredValue& mvAngleL1L2 = m_mapDataPoints[DP_ANGLE_VOLTAGE_L1L2];
	const MeasuredValue& mvAngleL1L3 = m_mapDataPoints[DP_ANGLE_VOLTAGE_L1L3];
	const double angleL2L3 = mvAngleL1L3.getValueNow() - mvAngleL1L2.getValueNow();

	const MeasuredValue& mvVoltageL1N = m_mapDataPoints[DP_VOLTAGE_L1N];
	const MeasuredValue& mvVoltageL2N = m_mapDataPoints[DP_VOLTAGE_L2N];
	const MeasuredValue& mvVoltageL3N = m_mapDataPoints[DP_VOLTAGE_L3N];

	MeasuredValue& mvVoltageL1L2 = m_mapDataPoints[DP_VOLTAGE_L1L2];
	MeasuredValue& mvVoltageL2L3 = m_mapDataPoints[DP_VOLTAGE_L2L3];
	MeasuredValue& mvVoltageL3L1 = m_mapDataPoints[DP_VOLTAGE_L3L1];

	const double voltageL1L2 = calcVoltageLL(mvVoltageL1N.getValueNow(), mvVoltageL2N.getValueNow(), mvAngleL1L2.getValueNow());
	const double voltageL2L3 = calcVoltageLL(mvVoltageL2N.getValueNow(), mvVoltageL3N.getValueNow(), angleL2L3);
	const double voltageL3L1 = calcVoltageLL(mvVoltageL3N.getValueNow(), mvVoltageL1N.getValueNow(), mvAngleL1L3.getValueNow());

	mvVoltageL1L2.updateValue(voltageL1L2);
	mvVoltageL2L3.updateValue(voltageL2L3);
	mvVoltageL3L1.updateValue(voltageL3L1);
}

void DataCollector::calcDerivedPowerFactor()
{
	// Calculate cos(phi) from phase angele
	const MeasuredValue& mvAngleCurrentL1 = m_mapDataPoints[DP_ANGLE_CURRENT_L1];
	const MeasuredValue& mvAngleCurrentL2 = m_mapDataPoints[DP_ANGLE_CURRENT_L2];
	const MeasuredValue& mvAngleCurrentL3 = m_mapDataPoints[DP_ANGLE_CURRENT_L3];

	MeasuredValue& mvPfL1 = m_mapDataPoints[DP_POWER_FACTOR_L1];
	MeasuredValue& mvPfL2 = m_mapDataPoints[DP_POWER_FACTOR_L2];
	MeasuredValue& mvPfL3 = m_mapDataPoints[DP_POWER_FACTOR_L3];

	const double pfL1 = calcPowerFactor(mvAngleCurrentL1.getValueNow());
	const double pfL2 = calcPowerFactor(mvAngleCurrentL2.getValueNow());
	const double pfL3 = calcPowerFactor(mvAngleCurrentL3.getValueNow());

	mvPfL1.updateValue(pfL1);
	mvPfL2.updateValue(pfL2);
	mvPfL3.updateValue(pfL3);
}

void DataCollector::calcDerivedReactivePower()
{
	// Calculate reactive power from active power and phase angle
	const MeasuredValue& mvActPowL1 = m_mapDataPoints[DP_ACTIVE_POWER_L1];
	const MeasuredValue& mvActPowL2 = m_mapDataPoints[DP_ACTIVE_POWER_L2];
	const MeasuredValue& mvActPowL3 = m_mapDataPoints[DP_ACTIVE_POWER_L3];
	const MeasuredValue& mvActPowTotal = m_mapDataPoints[DP_ACTIVE_POWER_TOTAL];

	const MeasuredValue& mvAngleCurrentL1 = m_mapDataPoints[DP_ANGLE_CURRENT_L1];
	const MeasuredValue& mvAngleCurrentL2 = m_mapDataPoints[DP_ANGLE_CURRENT_L2];
	const MeasuredValue& mvAngleCurrentL3 = m_mapDataPoints[DP_ANGLE_CURRENT_L3];

	MeasuredValue& mvReactPowL1 = m_mapDataPoints[DP_REACTIVE_POWER_L1];
	MeasuredValue& mvReactPowL2 = m_mapDataPoints[DP_REACTIVE_POWER_L2];
	MeasuredValue& mvReactPowL3 = m_mapDataPoints[DP_REACTIVE_POWER_L3];
	MeasuredValue& mvReactPowTotal = m_mapDataPoints[DP_REACTIVE_POWER_TOTAL];

	const double reactPowL1 = calcReactivePower(mvActPowL1.getValueNow(), mvAngleCurrentL1.getValueNow());
	const double reactPowL2 = calcReactivePower(mvActPowL2.getValueNow(), mvAngleCurrentL2.getValueNow());
	const double reactPowL3 = calcReactivePower(mvActPowL3.getValueNow(), mvAngleCurrentL3.getValueNow());
	const double reactPowTotal = reactPowL1 + reactPowL2 + reactPowL3;

	mvReactPowL1.updateValue(reactPowL1);
	mvReactPowL2.updateValue(reactPowL2);
	mvReactPowL3.updateValue(reactPowL3);
	mvReactPowTotal.updateValue(reactPowTotal);
}

void DataCollector::calcDerivedApparentPower()
{
	// Calculate apparent power and total power factor from active and reactive power
	const MeasuredValue& mvActPowL1 = m_mapDataPoints[DP_ACTIVE_POWER_L1];
	const MeasuredValue& mvActPowL2 = m_mapDataPoints[DP_ACTIVE_POWER_L2];
	const MeasuredValue& mvActPowL3 = m_mapDataPoints[DP_ACTIVE_POWER_L3];
	const MeasuredValue& mvActPowTotal = m_mapDataPoints[DP_ACTIVE_POWER_TOTAL];

	const MeasuredValue& mvReactPowL1 = m_mapDataPoints[DP_REACTIVE_POWER_L1];
	const MeasuredValue& mvReactPowL2 = m_mapDataPoints[DP_REACTIVE_POWER_L2];
	const MeasuredValue& mvReactPowL3 = m_mapDataPoints[DP_REACTIVE_POWER_L3];
	const MeasuredValue& mvReactPowTotal = m_mapDataPoints[DP_REACTIVE_POWER_TOTAL];

	MeasuredValue& mvAppPowL1 = m_mapDataPoints[DP_APPARENT_POWER_L1];
	MeasuredValue& mvAppPowL2 = m_mapDataPoints[DP_APPARENT_POWER_L2];
	MeasuredValue& mvAppPowL3 = m_mapDataPoints[DP_APPARENT_POWER_L3];
	MeasuredValue& mvAppPowTotal = m_mapDataPoints[DP_APPARENT_POWER_TOTAL];

	MeasuredValue& mvPfTotal = m_mapDataPoints[DP_POWER_FACTOR_TOTAL];

	const double appPowL1 = calcApparentPower(mvActPowL1.getValueNow(), mvReactPowL1.getValueNow());
	const double appPowL2 = calcApparentPower(mvActPowL2.getValueNow(), mvReactPowL2.getValueNow());
	const double appPowL3 = calcApparentPower(mvActPowL3.getValueNow(), mvReactPowL3.getValueNow());
	const double appPowTotal = appPowL1 + appPowL2 + appPowL3;

	mvAppPowL1.updateValue(appPowL1);
	mvAppPowL2.updateValue(appPowL2);
	mvAppPowL3.updateValue(appPowL3);
	mvAppPowTotal.updateValue(appPowTotal);

	const double cosphiTotal = calcPowerFactor(mvActPowTotal.getValueNow(), appPowTotal);
	mvPfTotal.updateValue(cosphiTotal);
}

double DataCollector::calcVoltageLL(const double& valueLN1, const double& valueLN2, const double& angleUU)
{
	// U_LL = sqrt(U_LN1^2 + U_LN2^2 - 2 * U_LN1 * U_LN2 * cos(angleUU))
	return std::sqrt(std::pow(valueLN1, 2) + std::pow(valueLN2, 2) - 2 * valueLN1 * valueLN2 * std::cos(angleUU * M_PI / 180.0));
}

double DataCollector::calcPowerFactor(const double& angleUI)
{
	// cos(phi) = cos(angleUI) = P / S
	return std::cos(angleUI * M_PI / 180.0);
}

double DataCollector::calcPowerFactor(const double& activePower, const double& apparentPower)
{
	double result = 1.0;

	if (apparentPower > 0.0)
		result = activePower / apparentPower;

	return result;
}

double DataCollector::calcReactivePower(const double& activePower, const double& angleUI)
{
	// Q = P * tan(angleUI)
	return activePower * std::tan(angleUI * M_PI / 180.0);
}

double DataCollector::calcApparentPower(const double& activePower, const double& reactivePower)
{
	// S = sqrt(P^2 + Q^2)
	return std::sqrt(std::pow(activePower, 2) + std::pow(reactivePower, 2));
}
