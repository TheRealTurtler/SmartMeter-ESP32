#pragma once

#include <stdint.h>


class MeasuredValue
{
public:
	MeasuredValue() = default;
	~MeasuredValue() = default;

	const double& getValueNow() const { return m_valueNow; }
	const double& getValueAverage() const { return m_valueAverage; }

	uint16_t getSampleCount() const { return m_sampleCount; }

	void updateValue(const double& newValue, bool calcAverage = true)
	{
		m_valueNow = newValue;
		++m_sampleCount;

		if (calcAverage)
			m_valueAverage += (newValue - m_valueAverage) / m_sampleCount;
		else
			m_valueAverage = newValue;
	}

	void resetAverage()
	{
		m_valueAverage = 0.0;
		m_sampleCount = 0;
	}

private:
	double m_valueNow = 0.0;
	double m_valueAverage = 0.0;

	uint32_t m_sampleCount = 0;
};
