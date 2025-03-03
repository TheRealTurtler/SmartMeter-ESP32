#pragma once

class Watchdog
{
public:
	Watchdog() = default;
	~Watchdog() = default;

	void init();
	void update();
};
