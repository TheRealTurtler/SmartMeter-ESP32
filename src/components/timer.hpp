#pragma once

#include <functional>
#include <chrono>
#include <vector>

class Timer
{
public:
	Timer();
	~Timer();

	typedef std::function<void(void)> callback_fc;

	void addCallback(const callback_fc& callback);

	void setSingleShot(bool singleshot) { m_singleshot = singleshot; }

	void setInterval(const std::chrono::nanoseconds& interval) { m_interval = interval; }

	void start();
	void start(const std::chrono::nanoseconds& interval);
	void stop();

	void update();

	bool isRunning() { return m_started; }

private:
	bool m_started = false;
	bool m_singleshot = false;

	std::vector<callback_fc> m_vecFuncCallbacks;

	std::chrono::nanoseconds m_interval = std::chrono::nanoseconds(0);
	std::chrono::steady_clock::time_point m_timeLast;
};
