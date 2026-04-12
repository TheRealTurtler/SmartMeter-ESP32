#pragma once

#include <string>


class DataCollector
{
public:
	DataCollector();
	~DataCollector() = default;

	std::string getJsonStatus() const;

private:

};
