#pragma once

#include <cstdint>
#include <string>


class DateTime
{
public:
	DateTime();

	static DateTime currentDateTime();
	static DateTime fromUSecsSinceEpoch(const int64_t& ts);
	static DateTime fromMSecsSinceEpoch(const int64_t& ts) { return fromUSecsSinceEpoch(ts * 1000); }
	static DateTime fromSecsSinceEpoch(const int64_t& ts) { return fromUSecsSinceEpoch(ts * 1000 * 1000); }

	static int64_t currentUSecsSinceEpoch();
	static int64_t currentMSecsSinceEpoch() { { return (currentUSecsSinceEpoch() / 1000); } }
	static int64_t currentSecsSinceEpoch() { return (currentUSecsSinceEpoch() / 1000 / 1000); }

	uint16_t year() const { return m_year; }
	uint8_t month() const { return m_month; }
	uint8_t day() const { return m_day; }

	uint8_t hour() const { return m_hour; }
	uint8_t minute() const { return m_minute; }
	uint8_t second() const { return m_second; }

	const int64_t& toUSecsSinceEpoch() const;
	int64_t toMSecsSinceEpoch() const { return (m_timestamp_us / 1000); }
	int64_t toSecsSinceEpoch() const { return (m_timestamp_us / 1000 / 1000); }

	bool isValid() const { return (m_timestamp_us >= 0); }

	DateTime addUSecs(const int64_t& us) const;
	DateTime addMSecs(const int64_t& ms) const { return addUSecs(ms * 1000); }
	DateTime addSecs(const int64_t& s) const { return addUSecs(s * 1000 * 1000); }

	int64_t uSecsTo(const DateTime& dt) const;
	int64_t mSecsTo(const DateTime& dt) const { return (uSecsTo(dt) / 1000); }
	int64_t secsTo(const DateTime& dt) const { return (uSecsTo(dt) / 1000 / 1000); }

	// For format description see strftime
	std::string toString(const std::string& format) const;

	bool operator==(const DateTime& other) const { return m_timestamp_us == other.m_timestamp_us; }
	bool operator!=(const DateTime& other) const { return !(*this == other); }
	bool operator<(const DateTime& other) const { return m_timestamp_us < other.m_timestamp_us; }
	bool operator>(const DateTime& other) const { return other < *this; }
	bool operator<=(const DateTime& other) const { return !(*this > other); }
	bool operator>=(const DateTime& other) const { return !(*this < other); }

private:
	int64_t m_timestamp_us = -1;

	uint16_t m_year = 0;
	uint8_t m_month = 0;
	uint8_t m_day = 0;

	uint8_t m_hour = 0;
	uint8_t m_minute = 0;
	uint8_t m_second = 0;

	void calcDateTimeFromTimestamp();

	struct tm getTimeInfo() const;
};
