#include "datetime.hpp"
#include <time.h>
#include <Arduino.h>


DateTime::DateTime()
{

}

DateTime DateTime::currentDateTime()
{
	const int64_t tsNow = currentUSecsSinceEpoch();
	return fromUSecsSinceEpoch(tsNow);
}

DateTime DateTime::fromUSecsSinceEpoch(const int64_t& ts)
{
	DateTime result;
	result.m_timestamp_us = ts;
	result.calcDateTimeFromTimestamp();

	return result;
}

int64_t DateTime::currentUSecsSinceEpoch()
{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);

	return (static_cast<int64_t>(tv_now.tv_sec) * 1000000L + tv_now.tv_usec);
}

const int64_t& DateTime::toUSecsSinceEpoch() const
{
	return m_timestamp_us;
}

DateTime DateTime::addUSecs(const int64_t& us) const
{
	return fromUSecsSinceEpoch(m_timestamp_us + us);
}

int64_t DateTime::uSecsTo(const DateTime& dt) const
{
	return (dt.toUSecsSinceEpoch() - m_timestamp_us);
}

std::string DateTime::toString(const std::string& format) const
{
	const struct tm timeinfo = getTimeInfo();

	char buffer[32];
	strftime(buffer, sizeof(buffer), format.c_str(), &timeinfo);

	return std::string(buffer);
}

void DateTime::calcDateTimeFromTimestamp()
{
	const struct tm timeinfo = getTimeInfo();

	m_year = timeinfo.tm_year + 1900;
	m_month = timeinfo.tm_mon + 1;
	m_day = timeinfo.tm_mday;

	m_hour = timeinfo.tm_hour;
	m_minute = timeinfo.tm_min;
	m_second = timeinfo.tm_sec;
}

struct tm DateTime::getTimeInfo() const
{
	const time_t ts = (m_timestamp_us / 1000 / 1000);

	struct tm timeinfo;
	localtime_r(&ts, &timeinfo);

	return timeinfo;
}
