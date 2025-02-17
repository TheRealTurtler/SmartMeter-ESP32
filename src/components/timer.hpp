#pragma once

#include <functional>

class Timer
{
public:
	Timer();
	~Timer();

	typedef std::function<void(void)> callback_fc;

	void setCallback(callback_fc callback);

	void setInterval_us(unsigned long interval);
	void setInterval_ms(unsigned long interval);
	void setInterval_s(unsigned long interval);

	void start();
	void stop();

	void update();

private:
	bool m_started = false;
	callback_fc m_callback;

	unsigned long m_interval = 0;
	unsigned long m_timeLast = 0;
};
