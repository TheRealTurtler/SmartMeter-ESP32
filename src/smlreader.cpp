#include "smlreader.hpp"
#include <Arduino.h>


const bool TEST = false;
const char* const testData = "1b1b1b1b010101017605010203046200620072650000010176010b0a014c4f4700058061aa05010203040b000000000000000000007262016500d456b701633991007605010203046200620072650000070177010b0a014c4f4700058061aa070100620affff7262016500d456b7f106770701006032010101010101044c4f470177070100600100ff010101010b0a014c4f4700058061aa0177070100010800ff65001c110401621e52ff690000000001047c000177070100020800ff0101621e52ff6900000000000169d80177070100100700ff0101621b52005900000000000000bc0177070100240700ff0101621b520059ffffffffffffff7e0177070100380700ff0101621b52005900000000000000ea01770701004c0700ff0101621b52005900000000000000540177070100200700ff0101622352ff6900000000000009120177070100340700ff0101622352ff6900000000000009090177070100480700ff0101622352ff69000000000000091301770701001f0700ff0101622152fe69000000000000001e0177070100330700ff0101622152fe6900000000000000720177070100470700ff0101622152fe6900000000000000250177070100510701ff0101620852ff5900000000000009610177070100510702ff0101620852ff5900000000000004b70177070100510704ff0101620852ff59000000000000075b017707010051070fff0101620852ff590000000000000072017707010051071aff0101620852ff59000000000000009d01770701000e0700ff0101622c52ff6900000000000001f4017707010000020000010101010330360177070100605a02010101010105184a937001010163d36e0076050102030462006200726500000201710163bbb500001b1b1b1b1a013bad";


const std::vector<SMLReader::ObisData> SMLReader::m_vecObisData =
{
	// OBIS Code								Data Point						Unit
	{ {0x01, 0x00, 16, 7, 0, 0xff}, 			DP_ACTIVE_POWER_TOTAL,			SML_WATT},
	{ {0x01, 0x00, 36, 7, 0, 0xff}, 			DP_ACTIVE_POWER_L1,				SML_WATT},
	{ {0x01, 0x00, 56, 7, 0, 0xff}, 			DP_ACTIVE_POWER_L2,				SML_WATT},
	{ {0x01, 0x00, 76, 7, 0, 0xff}, 			DP_ACTIVE_POWER_L3,				SML_WATT},
	{ {0x01, 0x00, 32, 7, 0, 0xff}, 			DP_VOLTAGE_L1N,					SML_VOLT},
	{ {0x01, 0x00, 52, 7, 0, 0xff}, 			DP_VOLTAGE_L2N,					SML_VOLT},
	{ {0x01, 0x00, 72, 7, 0, 0xff}, 			DP_VOLTAGE_L3N,					SML_VOLT},
	{ {0x01, 0x00, 31, 7, 0, 0xff}, 			DP_CURRENT_L1,					SML_AMPERE},
	{ {0x01, 0x00, 51, 7, 0, 0xff}, 			DP_CURRENT_L2,					SML_AMPERE},
	{ {0x01, 0x00, 71, 7, 0, 0xff}, 			DP_CURRENT_L3,					SML_AMPERE},
	{ {0x01, 0x00, 14, 7, 0, 0xff}, 			DP_FREQUENCY,					SML_HERTZ},
	{ {0x01, 0x00, 1,  8, 0, 0xff}, 			DP_ACTIVE_ENERGY_IMPORT,		SML_WATT_HOUR},
	{ {0x01, 0x00, 2,  8, 0, 0xff}, 			DP_ACTIVE_ENERGY_EXPORT,		SML_WATT_HOUR},
	{ {0x01, 0x00, 81, 7, 1, 0xff}, 			DP_ANGLE_VOLTAGE_L1L2,			SML_DEGREE},
	{ {0x01, 0x00, 81, 7, 2, 0xff}, 			DP_ANGLE_VOLTAGE_L3L1,			SML_DEGREE},
	{ {0x01, 0x00, 81, 7, 4, 0xff}, 			DP_ANGLE_CURRENT_L1,			SML_DEGREE},
	{ {0x01, 0x00, 81, 7, 15, 0xff}, 			DP_ANGLE_CURRENT_L2,			SML_DEGREE},
	{ {0x01, 0x00, 81, 7, 26, 0xff}, 			DP_ANGLE_CURRENT_L3,			SML_DEGREE},
};


SMLReader::SMLReader(DataCollector* const dc, HardwareSerial& serial, uint8_t pinRx, uint8_t pinTx):
	m_dc(dc),
	m_serial(serial),
	m_pinRx(pinRx),
	m_pinTx(pinTx)
{

}

void SMLReader::init()
{
	m_timeLastUpdate = std::chrono::steady_clock::now();

	Serial1.setRxBufferSize(2048);
	Serial1.begin(9600, SERIAL_8N1, m_pinRx, m_pinTx, true);
}

void SMLReader::update()
{
	if (TEST)
	{
		static auto timeLast = std::chrono::steady_clock::now();
		const auto timeNow = std::chrono::steady_clock::now();
		const auto timeDiff = (timeNow - timeLast);

		if (timeDiff >= std::chrono::seconds(2))
		{
			for (size_t testIdx = 0; testIdx < strlen(testData) - 1; testIdx += 2)
			{
				unsigned int received;
				sscanf(testData + testIdx, "%02hhx", &received);
				const unsigned char c = (received & 0xff);
				readByte(c);
			}

			timeLast = timeNow;
		}
	}
	else
	{
		while (Serial1.available())
		{
			const int received = Serial1.read();
			const unsigned char c = (received & 0xff);
			readByte(c);
		}
	}
}

bool SMLReader::readByte(unsigned char byte)
{
	bool result = true;

	const sml_states_t currentState = smlState(byte);

	switch (currentState)
	{
	case SML_START:
	{
		log_d(">>> Start of a new message! <<<");
		m_mapValues.clear();
		m_timeLastUpdate = std::chrono::steady_clock::now();
		break;
	}

	case SML_LISTEND:
	{
		for (const auto& it : m_vecObisData)
		{
			if (!smlOBISCheck(it.obis))
				continue;

			obisHandler(it);
			break;
		}
		break;
	}

	case SML_UNEXPECTED:
	{
		log_d("Unexpected byte: 0x%02hhx", byte);
		result = false;
		break;
	}

	case SML_FINAL:
	{
		log_i(">>> Successfully received a complete message! <<<");

		if (m_dc)
		{
			for (const auto& [dp, value] : m_mapValues)
			{
				bool calcAvg = true;

				if (dp == DP_ACTIVE_ENERGY_IMPORT || dp == DP_ACTIVE_ENERGY_EXPORT)
					calcAvg = false;

				m_dc->updateDatapoint(dp, value, calcAvg);
			}

			m_dc->calcDerivedValues();
			m_dc->setTimeLastUpdateSmartmeter(m_timeLastUpdate);
		}
		break;
	}

	default:
		break;
	}

	return result;
}

void SMLReader::obisHandler(const ObisData& obisValue)
{
	double val = 0.0;
	bool okVal = true;

	switch (obisValue.unit)
	{
	case SML_DEGREE:
		smlOBISDegree(val);
		break;
	case SML_WATT:
		smlOBISW(val);
		break;
	case SML_WATT_HOUR:
		smlOBISWh(val);
		break;
	case SML_AMPERE:
		smlOBISAmpere(val);
		break;
	case SML_VOLT:
		smlOBISVolt(val);
		break;
	case SML_HERTZ:
		smlOBISHertz(val);
		break;

	default:
		okVal = false;
		break;
	}

	log_d("OBIS: %d.%d.%d -> DP: %d, Value: %f OK: %d", obisValue.obis[2], obisValue.obis[3], obisValue.obis[4], obisValue.dp, val, okVal);

	if (okVal)
		m_mapValues[obisValue.dp] = val;
}
