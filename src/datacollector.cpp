#include "datacollector.hpp"
#include <cmath>
#include <algorithm>

// FIXME remove
#include <Arduino.h>


DataCollector::DataCollector(AVG_INTERVAL avgInterval):
	m_avgInterval_us(avgInterval * 60 * 1000 * 1000)
{

}

void DataCollector::init()
{
	auto cb = [this]() { this->callbackAverage(); };

	m_timerAverage.setCallback(cb);
	m_timerAverage.setSingleShot(true);
}

void DataCollector::start()
{
	m_dtLastAverage = roundDateTime(DateTime::currentDateTime());
	m_timerAverage.start_s(10);
}

void DataCollector::update()
{
	m_timerAverage.update();

	static unsigned int ctr = 0;

	if (++ctr % 1000 == 0 && false)
	{
		ctr = 0;

		Serial.print("Samples: ");
		Serial.println(m_dataSmartMeter.mapData[DP_ACTIVE_POWER_TOTAL].getSampleCount());

		Serial.print("MCU Usage: ");
		Serial.println(m_dataSystem.mapData[DP_MCU_USAGE].getValueAverage());
		Serial.print("WiFi RSSI: ");
		Serial.println(m_dataSystem.mapData[DP_WIFI_RSSI].getValueAverage());

		Serial.print("Voltage L1-N: ");
		Serial.print(m_dataSmartMeter.mapData[DP_VOLTAGE_L1N].getValueNow());
		Serial.print(" L2-N: ");
		Serial.print(m_dataSmartMeter.mapData[DP_VOLTAGE_L2N].getValueNow());
		Serial.print(" L3-N: ");
		Serial.println(m_dataSmartMeter.mapData[DP_VOLTAGE_L3N].getValueNow());

		Serial.print("Voltage L1-L2: ");
		Serial.print(m_dataSmartMeter.mapData[DP_VOLTAGE_L1L2].getValueNow());
		Serial.print(" L2-L3: ");
		Serial.print(m_dataSmartMeter.mapData[DP_VOLTAGE_L2L3].getValueNow());
		Serial.print(" L3-L1: ");
		Serial.println(m_dataSmartMeter.mapData[DP_VOLTAGE_L3L1].getValueNow());

		Serial.print("Active Power L1: ");
		Serial.print(m_dataSmartMeter.mapData[DP_ACTIVE_POWER_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_dataSmartMeter.mapData[DP_ACTIVE_POWER_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.print(m_dataSmartMeter.mapData[DP_ACTIVE_POWER_L3].getValueNow());
		Serial.print(" Total: ");
		Serial.println(m_dataSmartMeter.mapData[DP_ACTIVE_POWER_TOTAL].getValueNow());

		Serial.print("Reactive Power L1: ");
		Serial.print(m_dataSmartMeter.mapData[DP_REACTIVE_POWER_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_dataSmartMeter.mapData[DP_REACTIVE_POWER_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.print(m_dataSmartMeter.mapData[DP_REACTIVE_POWER_L3].getValueNow());
		Serial.print(" Total: ");
		Serial.println(m_dataSmartMeter.mapData[DP_REACTIVE_POWER_TOTAL].getValueNow());

		Serial.print("Apparent Power L1: ");
		Serial.print(m_dataSmartMeter.mapData[DP_APPARENT_POWER_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_dataSmartMeter.mapData[DP_APPARENT_POWER_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.print(m_dataSmartMeter.mapData[DP_APPARENT_POWER_L3].getValueNow());
		Serial.print(" Total: ");
		Serial.println(m_dataSmartMeter.mapData[DP_APPARENT_POWER_TOTAL].getValueNow());

		Serial.print("Power Factor L1: ");
		Serial.print(m_dataSmartMeter.mapData[DP_POWER_FACTOR_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_dataSmartMeter.mapData[DP_POWER_FACTOR_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.print(m_dataSmartMeter.mapData[DP_POWER_FACTOR_L3].getValueNow());
		Serial.print(" Total: ");
		Serial.println(m_dataSmartMeter.mapData[DP_POWER_FACTOR_TOTAL].getValueNow());

		Serial.print("Current L1: ");
		Serial.print(m_dataSmartMeter.mapData[DP_CURRENT_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_dataSmartMeter.mapData[DP_CURRENT_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.println(m_dataSmartMeter.mapData[DP_CURRENT_L3].getValueNow());

		Serial.print("Frequency: ");
		Serial.println(m_dataSmartMeter.mapData[DP_FREQUENCY].getValueNow());

		Serial.print("Angle U L1-L2: ");
		Serial.print(m_dataSmartMeter.mapData[DP_ANGLE_VOLTAGE_L1L2].getValueNow());
		Serial.print(" L1-L3: ");
		Serial.println(m_dataSmartMeter.mapData[DP_ANGLE_VOLTAGE_L3L1].getValueNow());

		Serial.print("Angle I L1: ");
		Serial.print(m_dataSmartMeter.mapData[DP_ANGLE_CURRENT_L1].getValueNow());
		Serial.print(" L2: ");
		Serial.print(m_dataSmartMeter.mapData[DP_ANGLE_CURRENT_L2].getValueNow());
		Serial.print(" L3: ");
		Serial.println(m_dataSmartMeter.mapData[DP_ANGLE_CURRENT_L3].getValueNow());

		Serial.print("Energy Import: ");
		Serial.print(m_dataSmartMeter.mapData[DP_ACTIVE_ENERGY_IMPORT].getValueNow());
		Serial.print(" Export: ");
		Serial.println(m_dataSmartMeter.mapData[DP_ACTIVE_ENERGY_EXPORT].getValueNow());
	}
}

void DataCollector::updateDatapoint(const DATA_POINT_SMARTMETER dp, const double& value, bool calcAverage)
{
	m_dataSmartMeter.mapData[dp].updateValue(value, calcAverage);
}

void DataCollector::updateDatapoint(const DATA_POINT_SYSTEM dp, const double& value, bool calcAverage)
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
	const DateTime dtNow = DateTime::currentDateTime();
	DateTime dtNext = m_dtLastAverage.addUSecs(m_avgInterval_us);

	if (dtNow >= dtNext)
	{
		Serial.println("Taking Averages...");

		if (m_cbSmartmeter)
			m_cbSmartmeter(dtNext, m_dataSmartMeter);

		if (m_cbSystem)
			m_cbSystem(dtNext, m_dataSystem);

		resetAverage();

		m_dtLastAverage = roundDateTime(dtNow);
		dtNext = m_dtLastAverage.addUSecs(m_avgInterval_us);
	}

	m_timerAverage.start_us(dtNow.uSecsTo(dtNext) + 1000);
}

void DataCollector::resetAverage()
{
	for (auto& [dp, value] : m_dataSmartMeter.mapData)
	{
		value.resetAverage();
	}
}

DateTime DataCollector::roundDateTime(const DateTime& dt) const
{
	int64_t ts = dt.toUSecsSinceEpoch() / m_avgInterval_us;
	ts *= m_avgInterval_us;

	return (DateTime::fromUSecsSinceEpoch(ts));
}

void DataCollector::calcDerivedVoltage()
{
	// Calculate L-L voltages from L-N voltages and phase angles
	const MeasuredValue& mvAngleL1L2 = m_dataSmartMeter.mapData[DP_ANGLE_VOLTAGE_L1L2];
	const MeasuredValue& mvAngleL3L1 = m_dataSmartMeter.mapData[DP_ANGLE_VOLTAGE_L3L1];
	MeasuredValue& mvAngleL2L3 = m_dataSmartMeter.mapData[DP_ANGLE_VOLTAGE_L2L3];

	mvAngleL2L3.updateValue(std::abs(mvAngleL3L1.getValueNow() - mvAngleL1L2.getValueNow()));

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
