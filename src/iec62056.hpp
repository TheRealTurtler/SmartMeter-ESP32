#pragma once

#include <Arduino.h>

#include "components/timer.hpp"
#include <string>


class IEC62065
{
public:
	IEC62065(HardwareSerial& serial, unsigned long interval_ms, uint8_t pinRx, uint8_t pinTx);
	~IEC62065();

	void init();
	void start();
	void stop();

	void update();

private:
	enum STATE
	{
		PAUSE,

		RX_ACTIVE,
		RX_TIMEOUT,
		RX_FINISHED,
		RX_ERROR,

		TX_INIT,
		TX_READOUT,
	};

	HardwareSerial& m_serial;

	const uint8_t m_pinRx;
	const uint8_t m_pinTx;

	Timer m_timerUpdate;
	Timer m_timerPause;
	Timer m_timerTimeout;

	STATE m_state = PAUSE;
	STATE m_stateTx = PAUSE;

	std::string m_strDataBuffer = "";

	std::string m_strManufactor = "";
	std::string m_strIdentifier = "";
	std::string m_strDataRaw = "";

	void updateState(STATE state);
	void updateStateTx(STATE state);

	void callbackUpdate();
	void callbackPause();
	void callbackTimeout();

	void statePause();
	void stateRxActive();
	void stateRxTimeout();
	void stateRxFinished();
	void stateRxError();
	void stateTxInit();
	void stateTxReadout();

	void sendRequest();
	void sendAck(char v, char z, char y);
	void sendAck();
	void sendNack();
	void sendProgrammingCommand(char c, char d, char* data);
	void sendReadout(int baud);

	bool decodeInit();
	bool decodeReadout();

	char calcBcc(const std::string& strData);
};
