#pragma once

#include "iec62056.hpp"
#include <string>


class DataCollector
{
public:
	DataCollector(const IEC62065& iec);
	~DataCollector() = default;

	std::string getJsonStatus() const;

	std::string getJsonIEC62065() const;
	const std::string& getRawIEC62065() const { return m_iec.getRaw(); }

private:
	const IEC62065& m_iec;
};
