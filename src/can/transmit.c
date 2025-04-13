// Header
#include "transmit.h"

// Conversions -----------------------------------------------------------------------------------------------------------------

// Voltage Values (V)
#define VOLTAGE_INVERSE_FACTOR				(1024.0f / 8.0f)
#define VOLTAGE_TO_WORD(voltage)			(uint16_t) ((voltage) * VOLTAGE_INVERSE_FACTOR)

// Temperature Values (C)
#define TEMPERATURE_INVERSE_FACTOR			(4096.0f / 256.0f)
#define TEMPERATURE_TO_WORD(temperature)	(uint16_t) ((temperature) * TEMPERATURE_INVERSE_FACTOR)

// Message IDs ----------------------------------------------------------------------------------------------------------------

#define STATUS_MESSAGE_ID			0x727
#define VOLTAGE_MESSAGE_BASE_ID		0x700
#define TEMPERATURE_MESSAGE_BASE_ID	0x718
#define SENSE_LINE_STATUS_BASE_ID	0x724

// Functions ------------------------------------------------------------------------------------------------------------------

msg_t transmitStatusMessage (CANDriver* driver, sysinterval_t timeout)
{
	bool undervoltage = ltc6811UndervoltageFault (ltcBottom);
	bool overvoltage = ltc6811OvervoltageFault (ltcBottom);
	bool senseLine = ltc6811OpenWireFault (ltcBottom);
	bool isoSpi = ltc6811IsospiFault (ltcBottom);
	bool selfTest = ltc6811SelfTestFault (ltcBottom);

	bool undertemperature = false;
	for (uint16_t ltcIndex = 0; ltcIndex < LTC_COUNT; ++ltcIndex)
		for (uint16_t thermistorIndex = 0; thermistorIndex < LTC6811_GPIO_COUNT; ++thermistorIndex)
			undertemperature |= thermistors [ltcIndex][thermistorIndex].undertemperatureFault;

	bool overtemperature = false;
	for (uint16_t ltcIndex = 0; ltcIndex < LTC_COUNT; ++ltcIndex)
		for (uint16_t thermistorIndex = 0; thermistorIndex < LTC6811_GPIO_COUNT; ++thermistorIndex)
			overtemperature |= thermistors [ltcIndex][thermistorIndex].overtemperatureFault;

	CANTxFrame frame =
	{
		.DLC	= 6,
		.IDE	= CAN_IDE_STD,
		.SID	= STATUS_MESSAGE_ID,
		.data8	=
		{
			undervoltage |
			(overvoltage << 1) |
			(undertemperature << 2) |
			(overtemperature << 3) |
			(senseLine << 4) |
			(isoSpi << 5) |
			(selfTest << 6)
		}
	};

	// IsoSPI faults
	for (uint8_t index = 0; index < LTC_COUNT; ++index)
		frame.data16 [1] |= (ltcs [index].state == LTC6811_STATE_FAILED || ltcs [index].state == LTC6811_STATE_PEC_ERROR) << index;

	// Self test faults
	for (uint8_t index = 0; index < LTC_COUNT; ++index)
		frame.data16 [2] |= (ltcs [index].state == LTC6811_STATE_SELF_TEST_FAULT) << index;

	return canTransmitTimeout (driver, CAN_ANY_MAILBOX, &frame, timeout);
}

msg_t transmitVoltageMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index)
{
	uint16_t ltcIndex = index / 2;
	uint8_t voltOffset = (index % 2) * 6;

	uint16_t voltages [6];
	bool undervoltage = false;
	bool overvoltage = false;
	for (uint8_t voltIndex = 0; voltIndex < 6; ++voltIndex)
	{
		voltages [voltIndex] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [voltOffset + voltIndex]);
		undervoltage |= ltcs [ltcIndex].undervoltageFaults [voltOffset + voltIndex];
		overvoltage |= ltcs [ltcIndex].overvoltageFaults [voltOffset + voltIndex];
	}

	CANTxFrame frame =
	{
		.DLC	= 8,
		.IDE	= CAN_IDE_STD,
		.SID	= VOLTAGE_MESSAGE_BASE_ID + index,
		.data8	=
		{
			voltages [0],
			(voltages [1] << 2) | ((voltages [0] >> 8) & 0b11),
			(voltages [2] << 4) | ((voltages [1] >> 6) & 0b1111),
			(voltages [3] << 6) | ((voltages [2] >> 4) & 0b111111),
			voltages [3] >> 2,
			voltages [4],
			(voltages [5] << 2) | ((voltages [4] >> 8) & 0b11),
			(overvoltage << 5) | (undervoltage << 4) | ((voltages [5] >> 6) & 0b1111)
		}
	};

	return canTransmitTimeout (driver, CAN_ANY_MAILBOX, &frame, timeout);
}

msg_t transmitTemperatureMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index)
{
	uint16_t temperatures [5];

	// TODO(Barach): Re-enable temperatures
	// for (uint8_t tempIndex = 0; tempIndex < 5; ++tempIndex)
	// 	temperatures [tempIndex] = TEMPERATURE_TO_WORD (thermistors [index][tempIndex].temperature);

	bool undertemperature = false;
	bool overtemperature = false;
	for (uint8_t tempIndex = 0; tempIndex < 5; ++tempIndex)
	{
		if (thermistors [index][tempIndex].resistance < 16380.0f)
			temperatures [tempIndex] = thermistors [index][tempIndex].resistance / 4.0f;
		else
			temperatures [tempIndex] = 4095;

		undertemperature |= thermistors [index][tempIndex].undertemperatureFault;
		overtemperature |= thermistors [index][tempIndex].overtemperatureFault;
	}

	CANTxFrame frame =
	{
		.DLC	= 8,
		.IDE	= CAN_IDE_STD,
		.SID	= TEMPERATURE_MESSAGE_BASE_ID + index,
		.data8	=
		{
			temperatures [0],
			(temperatures [1] << 4) | ((temperatures [0] >> 8) & 0b1111),
			temperatures [1] >> 4,
			temperatures [2],
			(temperatures [3] << 4) | ((temperatures [2] >> 8) & 0b1111),
			temperatures [3] >> 4,
			temperatures [4],
			(overtemperature << 5) | (undertemperature << 4) | ((temperatures [4] >> 8) & 0b1111)
		}
	};

	return canTransmitTimeout (driver, CAN_ANY_MAILBOX, &frame, timeout);
}

msg_t transmitSenseLineStatusMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index)
{
	uint16_t ltcIndex = index * 4;

	CANTxFrame frame =
	{
		.DLC	= 8,
		.IDE	= CAN_IDE_STD,
		.SID	= SENSE_LINE_STATUS_BASE_ID + index,
	};

	for (uint8_t bit = 0; bit < LTC6811_CELL_COUNT + 1; ++bit)
		frame.data16 [0] |= ltcs [index].openWireFaults [bit] << bit;

	if (LTC_COUNT > ltcIndex + 1)
		for (uint8_t bit = 0; bit < LTC6811_CELL_COUNT + 1; ++bit)
			frame.data16 [1] |= ltcs [index + 1].openWireFaults [bit] << bit;

	if (LTC_COUNT > ltcIndex + 2)
		for (uint8_t bit = 0; bit < LTC6811_CELL_COUNT + 1; ++bit)
			frame.data16 [2] |= ltcs [index + 2].openWireFaults [bit] << bit;

	if (LTC_COUNT > ltcIndex + 3)
		for (uint8_t bit = 0; bit < LTC6811_CELL_COUNT + 1; ++bit)
			frame.data16 [3] |= ltcs [index + 3].openWireFaults [bit] << bit;

	return canTransmitTimeout (driver, CAN_ANY_MAILBOX, &frame, timeout);
}