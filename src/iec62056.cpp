#include "iec62056.hpp"

const char C_ACK = 0x06;
const char C_NACK = 0x15;
const char C_STX = 0x02;
const char C_SOH = 0x01;
const char C_ETX = 0x03;
const char C_EOT = 0x04;


IEC62065::IEC62065(HardwareSerial& serial, unsigned long interval_ms, uint8_t pinRx, uint8_t pinTx):
	m_serial(serial),
	m_pinRx(pinRx),
	m_pinTx(pinTx)
{
	auto cbUpdate = [this]() { this->callbackUpdate(); };
	auto cbPause = [this]() { this->callbackPause(); };
	auto cbTimeout = [this]() { this->callbackTimeout(); };

	m_timerUpdate.setCallback(cbUpdate);
	m_timerUpdate.setInterval_ms(interval_ms);

	m_timerPause.setCallback(cbPause);
	m_timerPause.setInterval_ms(1000);

	m_timerTimeout.setCallback(cbTimeout);
	m_timerTimeout.setInterval_ms(500);
}

IEC62065::~IEC62065()
{
	stop();
}

void IEC62065::init()
{

}

void IEC62065::start()
{
	m_timerUpdate.start();
}

void IEC62065::stop()
{
	m_timerUpdate.stop();
	m_timerPause.stop();
	m_timerTimeout.stop();
}

void IEC62065::update()
{
	m_timerUpdate.update();
	m_timerPause.update();
	m_timerTimeout.update();
}

void IEC62065::updateState(STATE state)
{
	m_state = state;
}

void IEC62065::updateStateTx(STATE state)
{
	m_stateTx = state;
}

void IEC62065::callbackUpdate()
{
	switch (m_state)
	{
	case PAUSE:
		statePause();
		break;

	case RX_ACTIVE:
		stateRxActive();
		break;

	case RX_TIMEOUT:
		stateRxTimeout();
		break;

	case RX_FINISHED:
		stateRxFinished();
		break;

	case TX_INIT:
		stateTxInit();
		break;

	case TX_READOUT:
		stateTxReadout();
		break;

	default:
		break;
	}
}

void IEC62065::callbackPause()
{
	if (m_state == PAUSE)
		updateState(TX_INIT);

	m_timerPause.stop();
}

void IEC62065::callbackTimeout()
{
	if (m_state == RX_ACTIVE)
		updateState(RX_TIMEOUT);

	m_timerTimeout.stop();
}

void IEC62065::statePause()
{
	if (!m_timerPause.isRunning())
		m_timerPause.start();
}

void IEC62065::stateRxActive()
{
	while (m_serial.available())
	{
		const int received = m_serial.read();

		if (received >= 0)
		{
			m_timerTimeout.start();
			m_strDataBuffer.push_back(received & 0xff);
		}

		if (m_strDataBuffer.ends_with("\r\n"))
		{
			m_timerTimeout.stop();
			updateState(RX_FINISHED);
		}
	}
}

void IEC62065::stateRxTimeout()
{
	updateState(PAUSE);

	Serial.println("Timeout! -> Re-Initialising...");
}

void IEC62065::stateRxFinished()
{
	bool okReceived = false;

	switch (m_stateTx)
	{
	case TX_INIT:
		if (decodeInit())
		{
			okReceived = true;
			updateState(TX_READOUT);
		}
		break;

	case TX_READOUT:
		if (decodeReadout())
		{
			okReceived = true;
			updateState(PAUSE);
		}
		break;

	default:
		break;
	}

	m_strDataBuffer = "";

	if (!okReceived)
		updateState(RX_ERROR);
}

void IEC62065::stateRxError()
{
	updateState(PAUSE);

	Serial.print("Error! Received: ");
	Serial.print(m_strDataBuffer.data());
	Serial.println();
}

void IEC62065::stateTxInit()
{
	m_strDataBuffer = "";

	m_serial.end();
	m_serial.begin(300, SERIAL_7E1, m_pinRx, m_pinTx, true);

	sendRequest();

	m_timerTimeout.start();

	updateState(RX_ACTIVE);
	updateStateTx(TX_INIT);
}

void IEC62065::stateTxReadout()
{
	m_strDataBuffer = "";

	m_serial.end();
	m_serial.begin(9600, SERIAL_7E1, m_pinRx, m_pinTx, true);

	sendReadout(9600);

	m_timerTimeout.start();

	updateState(RX_ACTIVE);
	updateStateTx(TX_READOUT);
}

void IEC62065::sendRequest()
{
	m_serial.write("/?!\r\n");
}

void IEC62065::sendAck(char v, char z, char y)
{
	size_t idx = 0;

	char buffer[7];
	buffer[idx++] = C_ACK;	// ACK
	buffer[idx++] = v;		// Protocol Control
	buffer[idx++] = z;		// Baud Rate Identification
	buffer[idx++] = y;		// Mode Control
	buffer[idx++] = '\r';	// CR
	buffer[idx++] = '\n';	// LF
	buffer[idx++] = 0x00;

	m_serial.write(buffer);
}

void IEC62065::sendAck()
{
	m_serial.write(C_ACK);
}

void IEC62065::sendNack()
{
	m_serial.write(C_NACK);
}

void IEC62065::sendProgrammingCommand(char c, char d, char* data)
{
	char bcc = 0;	// TODO

	size_t idx = 0;

	char buffer[7];			// TODO
	buffer[idx++] = C_SOH;	// Start of Header
	buffer[idx++] = c;		// Command Message
	buffer[idx++] = d;		// Command Type
	buffer[idx++] = C_STX;	// Frame Start

	// TODO: Data

	buffer[idx++] = C_ETX;	// End
	buffer[idx++] = bcc;	// Block Check
	buffer[idx++] = 0x00;
}

void IEC62065::sendReadout(int baud)
{
	const int factor = baud / 300 - 1;

	if (factor < 0 || factor > 6)
	{
		Serial.print("Invalid Baudrate: ");
		Serial.print(baud);
		Serial.println();
	}

	char v = '0';
	char z = '0' + factor;
	char y = '0';

	sendAck(v, z, y);
}

bool IEC62065::decodeInit()
{
	bool result = false;

	// / X X X Z \ W ID CR LF
	// X: Manufacturer
	// Z: Baud Rate
	// \: Delimiter - optional
	// W: Mode - optional

	if (m_strDataBuffer.starts_with("/") && m_strDataBuffer.ends_with("\r\n"))
	{
		std::string strManuf = m_strDataBuffer.substr(1, 3);
		std::string strId = m_strDataBuffer.substr(5, m_strDataBuffer.size() - 7);

		if (strId.starts_with("\\"))
			strId = strId.substr(2);

		if (!strManuf.empty() && !strId.empty())
		{
			m_strManufactor = std::move(strManuf);
			m_strIdentifier = std::move(strId);
			result = true;
		}
	}

	return result;
}

bool IEC62065::decodeReadout()
{
	bool result = false;

	// STX DATA ! CR LF ETX BCC

	if (m_strDataBuffer.starts_with(C_STX))
	{
		size_t idx = m_strDataBuffer.find("!\r\n");
		std::string strDataRaw = m_strDataBuffer.substr(1, idx - 1);

		char bccReceived = m_strDataBuffer.back();
		char bccCalculated = calcBcc(strDataRaw);

		if (bccReceived == bccCalculated)
		{
			m_strDataRaw = std::move(strDataRaw);
			result = true;
		}
	}

	return result;
}

char IEC62065::calcBcc(const std::string& strData)
{
	char result = 0;

	for (const auto& c : strData)
	{
		result ^= c;
	}

	return result;
}
