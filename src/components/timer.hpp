#pragma once

#include <functional>

class Timer
{
public:
	Timer();
	~Timer();

	typedef std::function<void(void)> callback_fc;

	void setCallback(callback_fc callback);

	void setSingleShot(bool singleshot);

	void setInterval_us(const unsigned long& interval);
	void setInterval_ms(const unsigned long& interval);
	void setInterval_s(const unsigned long& interval);

	void start();
	void start_us(const unsigned long& interval);
	void start_ms(const unsigned long& interval);
	void start_s(const unsigned long& interval);
	void stop();

	void update();

	bool isRunning() { return m_started; }

private:
	bool m_started = false;
	bool m_singleshot = false;

	callback_fc m_callback;

	unsigned long m_interval = 0;
	unsigned long m_timeLast = 0;
};
