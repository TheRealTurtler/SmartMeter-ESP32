#include "smlreader.hpp"

#include <Arduino.h>
#include <sml.h>


const bool TEST = true;
const char* const testData = "1b1b1b1b010101017605010203046200620072650000010176010b0a014c4f4700058061aa05010203040b000000000000000000007262016500d456b701633991007605010203046200620072650000070177010b0a014c4f4700058061aa070100620affff7262016500d456b7f106770701006032010101010101044c4f470177070100600100ff010101010b0a014c4f4700058061aa0177070100010800ff65001c110401621e52ff690000000001047c000177070100020800ff0101621e52ff6900000000000169d80177070100100700ff0101621b52005900000000000000bc0177070100240700ff0101621b520059ffffffffffffff7e0177070100380700ff0101621b52005900000000000000ea01770701004c0700ff0101621b52005900000000000000540177070100200700ff0101622352ff6900000000000009120177070100340700ff0101622352ff6900000000000009090177070100480700ff0101622352ff69000000000000091301770701001f0700ff0101622152fe69000000000000001e0177070100330700ff0101622152fe6900000000000000720177070100470700ff0101622152fe6900000000000000250177070100510701ff0101620852ff5900000000000009610177070100510702ff0101620852ff5900000000000004b70177070100510704ff0101620852ff59000000000000075b017707010051070fff0101620852ff590000000000000072017707010051071aff0101620852ff59000000000000009d01770701000e0700ff0101622c52ff6900000000000001f4017707010000020000010101010330360177070100605a02010101010105184a937001010163d36e0076050102030462006200726500000201710163bbb500001b1b1b1b1a013bad";


#define MAX_STR_MANUF 5
unsigned char manuf[MAX_STR_MANUF];

void Manufacturer() { smlOBISManufacturer(manuf, MAX_STR_MANUF); }

void actPow()
{
	double val;
	smlOBISW(val);

	Serial.print("Wirkleistung Gesamt: ");
	Serial.println(val);
}

void actPowL1()
{
	double val;
	smlOBISW(val);

	Serial.print("Wirkleistung L1: ");
	Serial.println(val);
}

void actPowL2()
{
	double val;
	smlOBISW(val);

	Serial.print("Wirkleistung L2: ");
	Serial.println(val);
}

void actPowL3()
{
	double val;
	smlOBISW(val);

	Serial.print("Wirkleistung L3: ");
	Serial.println(val);
}

void voltL1()
{
	double val;
	smlOBISVolt(val);

	Serial.print("Spannung L1: ");
	Serial.println(val);
}

void voltL2()
{
	double val;
	smlOBISVolt(val);

	Serial.print("Spannung L2: ");
	Serial.println(val);
}

void voltL3()
{
	double val;
	smlOBISVolt(val);

	Serial.print("Spannung L3: ");
	Serial.println(val);
}

void currentL1()
{
	double val;
	smlOBISAmpere(val);

	Serial.print("Strom L1: ");
	Serial.println(val);
}

void currentL2()
{
	double val;
	smlOBISAmpere(val);

	Serial.print("Strom L2: ");
	Serial.println(val);
}

void currentL3()
{
	double val;
	smlOBISAmpere(val);

	Serial.print("Strom L3: ");
	Serial.println(val);
}

void freq()
{
	double val;
	smlOBISHertz(val);

	Serial.print("Frequenz: ");
	Serial.println(val);
}

void energyImport()
{
	double val;
	smlOBISWh(val);

	Serial.print("Energie IMP: ");
	Serial.println(val);
}

void energyExport()
{
	double val;
	smlOBISWh(val);

	Serial.print("Energie EXP: ");
	Serial.println(val);
}

void angleVoltageL2L1()
{
	double val;
	smlOBISDegree(val);

	Serial.print("Winkel U L2-L1: ");
	Serial.println(val);
}

void angleVoltageL3L1()
{
	double val;
	smlOBISDegree(val);

	Serial.print("Winkel U L3-L1: ");
	Serial.println(val);
}

void angleCurrentL1()
{
	double val;
	smlOBISDegree(val);

	Serial.print("Winkel I L1 - U L1: ");
	Serial.print(val);
	Serial.print(" -> Cos: ");
	Serial.println(cos(val * M_2_PI / 180.0));
}

void angleCurrentL2()
{
	double val;
	smlOBISDegree(val);

	Serial.print("Winkel I L2 - U L2: ");
	Serial.print(val);
	Serial.print(" -> Cos: ");
	Serial.println(cos(val * M_2_PI / 180.0));
}

void angleCurrentL3()
{
	double val;
	smlOBISDegree(val);

	Serial.print("Winkel I L3 - U L3: ");
	Serial.print(val);
	Serial.print(" -> Cos: ");
	Serial.println(cos(val * M_2_PI / 180.0));
}

const std::vector<SMLReader::ObisHandler> SMLReader::m_vecObisHandlers =
{
	// OBIS Code								Handler
	{ {0x81, 0x81, 0xc7, 0x82, 0x03, 0xff},		&Manufacturer},
	{ {0x01, 0x00, 16, 7, 0, 0xff}, 			&actPow},
	{ {0x01, 0x00, 36, 7, 0, 0xff}, 			&actPowL1},
	{ {0x01, 0x00, 56, 7, 0, 0xff}, 			&actPowL2},
	{ {0x01, 0x00, 76, 7, 0, 0xff}, 			&actPowL3},
	{ {0x01, 0x00, 32, 7, 0, 0xff}, 			&voltL1},
	{ {0x01, 0x00, 52, 7, 0, 0xff}, 			&voltL2},
	{ {0x01, 0x00, 72, 7, 0, 0xff}, 			&voltL3},
	{ {0x01, 0x00, 31, 7, 0, 0xff}, 			&currentL1},
	{ {0x01, 0x00, 51, 7, 0, 0xff}, 			&currentL2},
	{ {0x01, 0x00, 71, 7, 0, 0xff}, 			&currentL3},
	{ {0x01, 0x00, 14, 7, 0, 0xff}, 			&freq},
	{ {0x01, 0x00, 1,  8, 0, 0xff}, 			&energyImport},
	{ {0x01, 0x00, 2,  8, 0, 0xff}, 			&energyExport},
	{ {0x01, 0x00, 81, 7, 1, 0xff}, 			&angleVoltageL2L1},
	{ {0x01, 0x00, 81, 7, 2, 0xff}, 			&angleVoltageL3L1},
	{ {0x01, 0x00, 81, 7, 4, 0xff}, 			&angleCurrentL1},
	{ {0x01, 0x00, 81, 7, 15, 0xff}, 			&angleCurrentL2},
	{ {0x01, 0x00, 81, 7, 26, 0xff}, 			&angleCurrentL3},
};

SMLReader::SMLReader(HardwareSerial& serial, uint8_t pinRx, uint8_t pinTx):
	m_serial(serial),
	m_pinRx(pinRx),
	m_pinTx(pinTx)
{

}

void SMLReader::init()
{
	Serial1.begin(9600, SERIAL_8N1, m_pinRx, m_pinTx, true);
}

void SMLReader::update()
{
	if (TEST)
	{
		static int a = 0;
		a++;

		if (a % 1000 != 0)
			return;

		Serial.println("------------------------------------");

		for (int testIdx = 0; testIdx < 1320; testIdx += 2)
		{
			unsigned int u;
			sscanf(testData + testIdx, "%02x", &u);

			if (!readByte(u))
				break;
		}
	}
	else
	{
		while (Serial1.available())
		{
			const int received = Serial1.read();

			if (received >= 0)
			{
				const unsigned char c = received & 0xff;

				readByte(c);

				// FIXME remove
				//char buffer[3] = { 0 };
				//sprintf(buffer, "%02x", received);
				//Serial.print(buffer);
			}
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
		// Reset variables
		manuf[0] = 0;
		break;

	case SML_LISTEND:
		// Search for last received OBIS code in the handler list and call the handler if found
		for (const auto& it : m_vecObisHandlers)
		{
			if (!smlOBISCheck(it.obis))
				continue;

			if (it.handler)
				it.handler();

			break;
		}

		break;

	case SML_UNEXPECTED:
	{
		char buffer[3] = { 0 };
		sprintf(buffer, "%02x", byte);

		Serial.print(F(">>> Unexpected byte: "));
		Serial.print(buffer);
		Serial.println();

		result = false;
	}
	break;

	case SML_FINAL:
	{
		Serial.println(F(">>> Successfully received a complete message!"));
	}

	default:
		break;
	}

	return result;
}
