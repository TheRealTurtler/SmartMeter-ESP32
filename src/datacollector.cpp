#include "datacollector.hpp"
#include <Arduino.h>
#include <cmath>
#include <algorithm>


DataCollector::DataCollector(const std::chrono::milliseconds& avgInterval):
	m_avgInterval(avgInterval)
{

}

void DataCollector::init()
{
	auto cb = [this]() { this->callbackAverage(); };

	m_timerAverage.addCallback(cb);
	m_timerAverage.setSingleShot(true);
}

void DataCollector::start()
{
	const auto timeNow = std::chrono::system_clock::now();
	m_timeLastAverage = roundTimePoint(timeNow);

	m_timerAverage.start(std::chrono::seconds(10));
}

void DataCollector::update()
{
	m_timerAverage.update();
}

void DataCollector::updateDatapoint(const DATA_POINT_SMARTMETER dp, double value, bool calcAverage)
{
	if (dp == DP_ANGLE_VOLTAGE_L1L2
		|| dp == DP_ANGLE_VOLTAGE_L2L3
		|| dp == DP_ANGLE_VOLTAGE_L3L1)
	{
		if (value > 180.0)
			value = 360.0 - value;
	}

	m_dataSmartMeter.mapData[dp].updateValue(value, calcAverage);
}

void DataCollector::updateDatapoint(const DATA_POINT_SYSTEM dp, double value, bool calcAverage)
{
	m_dataSystem.mapData[dp].updateValue(value, calcAverage);
}

void DataCollector::calcDerivedValues()
{
	calcDerivedVoltage();
	calcDerivedPowerFactor();
	calcDerivedReactivePower();
	calcDerivedApparentPower();
}

void DataCollector::callbackAverage()
{
	const auto timeNow = std::chrono::system_clock::now();
	auto timeNext = (m_timeLastAverage + m_avgInterval);

	if (timeNow >= timeNext)
	{
		log_i("Taking Averages...");

		if (m_cbSmartmeter)
			m_cbSmartmeter(timeNext, m_dataSmartMeter);

		if (m_cbSystem)
			m_cbSystem(timeNext, m_dataSystem);

		resetAverage();
	}

	m_timeLastAverage = roundTimePoint(timeNow);
	timeNext = (m_timeLastAverage + m_avgInterval);

	const auto timeDiff = (timeNext - timeNow);
	m_timerAverage.start(timeDiff + std::chrono::milliseconds(1));
}

void DataCollector::resetAverage()
{
	for (auto& [dp, value] : m_dataSmartMeter.mapData)
	{
		value.resetAverage();
	}
}

std::chrono::system_clock::time_point DataCollector::roundTimePoint(const std::chrono::system_clock::time_point& tp) const
{
	const auto timeRounded = (tp.time_since_epoch() / m_avgInterval) * m_avgInterval;
	return std::chrono::system_clock::time_point(timeRounded);
}

void DataCollector::calcDerivedVoltage()
{
	// Calculate L-L voltages from L-N voltages and phase angles
	const MeasuredValue& mvAngleL1L2 = m_dataSmartMeter.mapData[DP_ANGLE_VOLTAGE_L1L2];
	const MeasuredValue& mvAngleL3L1 = m_dataSmartMeter.mapData[DP_ANGLE_VOLTAGE_L3L1];
	MeasuredValue& mvAngleL2L3 = m_dataSmartMeter.mapData[DP_ANGLE_VOLTAGE_L2L3];

	mvAngleL2L3.updateValue(360.0 - mvAngleL3L1.getValueNow() - mvAngleL1L2.getValueNow());

	const MeasuredValue& mvVoltageL1N = m_dataSmartMeter.mapData[DP_VOLTAGE_L1N];
	const MeasuredValue& mvVoltageL2N = m_dataSmartMeter.mapData[DP_VOLTAGE_L2N];
	const MeasuredValue& mvVoltageL3N = m_dataSmartMeter.mapData[DP_VOLTAGE_L3N];

	MeasuredValue& mvVoltageL1L2 = m_dataSmartMeter.mapData[DP_VOLTAGE_L1L2];
	MeasuredValue& mvVoltageL2L3 = m_dataSmartMeter.mapData[DP_VOLTAGE_L2L3];
	MeasuredValue& mvVoltageL3L1 = m_dataSmartMeter.mapData[DP_VOLTAGE_L3L1];

	const double voltageL1L2 = calcVoltageLL(mvVoltageL1N.getValueNow(), mvVoltageL2N.getValueNow(), mvAngleL1L2.getValueNow());
	const double voltageL2L3 = calcVoltageLL(mvVoltageL2N.getValueNow(), mvVoltageL3N.getValueNow(), mvAngleL2L3.getValueNow());
	const double voltageL3L1 = calcVoltageLL(mvVoltageL3N.getValueNow(), mvVoltageL1N.getValueNow(), mvAngleL3L1.getValueNow());

	mvVoltageL1L2.updateValue(voltageL1L2);
	mvVoltageL2L3.updateValue(voltageL2L3);
	mvVoltageL3L1.updateValue(voltageL3L1);

	log_d("Angle L1-L2: %f | L2-L3: %f | L3-L1: %f", mvAngleL1L2.getValueNow(), mvAngleL2L3.getValueNow(), mvAngleL3L1.getValueNow());
	log_d("Voltage L1-N: %f | L2-N: %f | L3-N: %f", mvVoltageL1N.getValueNow(), mvVoltageL2N.getValueNow(), mvVoltageL3N.getValueNow());
	log_d("Voltage L1-L2: %f | L2-L3: %f | L3-L1: %f", mvVoltageL1L2.getValueNow(), mvVoltageL2L3.getValueNow(), mvVoltageL3L1.getValueNow());
}

void DataCollector::calcDerivedPowerFactor()
{
	// Calculate cos(phi) from phase angele
	const MeasuredValue& mvAngleCurrentL1 = m_dataSmartMeter.mapData[DP_ANGLE_CURRENT_L1];
	const MeasuredValue& mvAngleCurrentL2 = m_dataSmartMeter.mapData[DP_ANGLE_CURRENT_L2];
	const MeasuredValue& mvAngleCurrentL3 = m_dataSmartMeter.mapData[DP_ANGLE_CURRENT_L3];

	MeasuredValue& mvPfL1 = m_dataSmartMeter.mapData[DP_POWER_FACTOR_L1];
	MeasuredValue& mvPfL2 = m_dataSmartMeter.mapData[DP_POWER_FACTOR_L2];
	MeasuredValue& mvPfL3 = m_dataSmartMeter.mapData[DP_POWER_FACTOR_L3];

	const double pfL1 = calcPowerFactor(mvAngleCurrentL1.getValueNow());
	const double pfL2 = calcPowerFactor(mvAngleCurrentL2.getValueNow());
	const double pfL3 = calcPowerFactor(mvAngleCurrentL3.getValueNow());

	mvPfL1.updateValue(pfL1);
	mvPfL2.updateValue(pfL2);
	mvPfL3.updateValue(pfL3);

	log_d("Angle Current L1: %f | L2: %f | L3: %f", mvAngleCurrentL1.getValueNow(), mvAngleCurrentL2.getValueNow(), mvAngleCurrentL3.getValueNow());
	log_d("Power Factor L1: %f | L2: %f | L3: %f", mvPfL1.getValueNow(), mvPfL2.getValueNow(), mvPfL3.getValueNow());
}

void DataCollector::calcDerivedReactivePower()
{
	// Calculate reactive power from active power and phase angle
	const MeasuredValue& mvActPowL1 = m_dataSmartMeter.mapData[DP_ACTIVE_POWER_L1];
	const MeasuredValue& mvActPowL2 = m_dataSmartMeter.mapData[DP_ACTIVE_POWER_L2];
	const MeasuredValue& mvActPowL3 = m_dataSmartMeter.mapData[DP_ACTIVE_POWER_L3];
	const MeasuredValue& mvActPowTotal = m_dataSmartMeter.mapData[DP_ACTIVE_POWER_TOTAL];

	const MeasuredValue& mvAngleCurrentL1 = m_dataSmartMeter.mapData[DP_ANGLE_CURRENT_L1];
	const MeasuredValue& mvAngleCurrentL2 = m_dataSmartMeter.mapData[DP_ANGLE_CURRENT_L2];
	const MeasuredValue& mvAngleCurrentL3 = m_dataSmartMeter.mapData[DP_ANGLE_CURRENT_L3];

	MeasuredValue& mvReactPowL1 = m_dataSmartMeter.mapData[DP_REACTIVE_POWER_L1];
	MeasuredValue& mvReactPowL2 = m_dataSmartMeter.mapData[DP_REACTIVE_POWER_L2];
	MeasuredValue& mvReactPowL3 = m_dataSmartMeter.mapData[DP_REACTIVE_POWER_L3];
	MeasuredValue& mvReactPowTotal = m_dataSmartMeter.mapData[DP_REACTIVE_POWER_TOTAL];

	const double reactPowL1 = calcReactivePower(mvActPowL1.getValueNow(), mvAngleCurrentL1.getValueNow());
	const double reactPowL2 = calcReactivePower(mvActPowL2.getValueNow(), mvAngleCurrentL2.getValueNow());
	const double reactPowL3 = calcReactivePower(mvActPowL3.getValueNow(), mvAngleCurrentL3.getValueNow());
	const double reactPowTotal = reactPowL1 + reactPowL2 + reactPowL3;

	mvReactPowL1.updateValue(reactPowL1);
	mvReactPowL2.updateValue(reactPowL2);
	mvReactPowL3.updateValue(reactPowL3);
	mvReactPowTotal.updateValue(reactPowTotal);

	log_d("Angle Current L1: %f | L2: %f | L3: %f", mvAngleCurrentL1.getValueNow(), mvAngleCurrentL2.getValueNow(), mvAngleCurrentL3.getValueNow());
	log_d("Active Power L1: %f | L2: %f | L3: %f | Total: %f", mvActPowL1.getValueNow(), mvActPowL2.getValueNow(), mvActPowL3.getValueNow(), mvActPowTotal.getValueNow());
	log_d("Reactive Power L1: %f | L2: %f | L3: %f | Total: %f", mvReactPowL1.getValueNow(), mvReactPowL2.getValueNow(), mvReactPowL3.getValueNow(), mvReactPowTotal.getValueNow());
}

void DataCollector::calcDerivedApparentPower()
{
	// Calculate apparent power and total power factor from active and reactive power
	const MeasuredValue& mvActPowL1 = m_dataSmartMeter.mapData[DP_ACTIVE_POWER_L1];
	const MeasuredValue& mvActPowL2 = m_dataSmartMeter.mapData[DP_ACTIVE_POWER_L2];
	const MeasuredValue& mvActPowL3 = m_dataSmartMeter.mapData[DP_ACTIVE_POWER_L3];
	const MeasuredValue& mvActPowTotal = m_dataSmartMeter.mapData[DP_ACTIVE_POWER_TOTAL];

	const MeasuredValue& mvReactPowL1 = m_dataSmartMeter.mapData[DP_REACTIVE_POWER_L1];
	const MeasuredValue& mvReactPowL2 = m_dataSmartMeter.mapData[DP_REACTIVE_POWER_L2];
	const MeasuredValue& mvReactPowL3 = m_dataSmartMeter.mapData[DP_REACTIVE_POWER_L3];
	const MeasuredValue& mvReactPowTotal = m_dataSmartMeter.mapData[DP_REACTIVE_POWER_TOTAL];

	MeasuredValue& mvAppPowL1 = m_dataSmartMeter.mapData[DP_APPARENT_POWER_L1];
	MeasuredValue& mvAppPowL2 = m_dataSmartMeter.mapData[DP_APPARENT_POWER_L2];
	MeasuredValue& mvAppPowL3 = m_dataSmartMeter.mapData[DP_APPARENT_POWER_L3];
	MeasuredValue& mvAppPowTotal = m_dataSmartMeter.mapData[DP_APPARENT_POWER_TOTAL];

	MeasuredValue& mvPfTotal = m_dataSmartMeter.mapData[DP_POWER_FACTOR_TOTAL];

	const double appPowL1 = calcApparentPower(mvActPowL1.getValueNow(), mvReactPowL1.getValueNow());
	const double appPowL2 = calcApparentPower(mvActPowL2.getValueNow(), mvReactPowL2.getValueNow());
	const double appPowL3 = calcApparentPower(mvActPowL3.getValueNow(), mvReactPowL3.getValueNow());
	const double appPowTotal = calcApparentPower(mvActPowTotal.getValueNow(), mvReactPowTotal.getValueNow());

	mvAppPowL1.updateValue(appPowL1);
	mvAppPowL2.updateValue(appPowL2);
	mvAppPowL3.updateValue(appPowL3);
	mvAppPowTotal.updateValue(appPowTotal);

	const double cosphiTotal = calcPowerFactor(mvActPowTotal.getValueNow(), appPowTotal);
	mvPfTotal.updateValue(cosphiTotal);

	log_d("Apparent Power L1: %f | L2: %f | L3: %f | Total: %f", mvAppPowL1.getValueNow(), mvAppPowL2.getValueNow(), mvAppPowL3.getValueNow(), mvAppPowTotal.getValueNow());
	log_d("Power Factor Total: %f", mvPfTotal.getValueNow());
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
