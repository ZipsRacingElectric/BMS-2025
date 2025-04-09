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
	bool overvoltage = false;
	for (uint16_t ltcIndex = 0; ltcIndex < LTC_COUNT; ++ltcIndex)
		for (uint8_t cellIndex = 0; cellIndex < LTC6811_CELL_COUNT; ++cellIndex)
			if (ltcs [ltcIndex].overvoltageFaults [cellIndex])
				overvoltage = true;

	bool undervoltage = false;
	for (uint16_t ltcIndex = 0; ltcIndex < LTC_COUNT; ++ltcIndex)
		for (uint8_t cellIndex = 0; cellIndex < LTC6811_CELL_COUNT; ++cellIndex)
			if (ltcs [ltcIndex].undervoltageFaults [cellIndex])
				undervoltage = true;

	bool senseLine = false;
	for (uint16_t ltcIndex = 0; ltcIndex < LTC_COUNT; ++ltcIndex)
		for (uint8_t cellIndex = 0; cellIndex < LTC6811_CELL_COUNT + 1; ++cellIndex)
			if (ltcs [ltcIndex].openWireFaults [cellIndex])
				senseLine = true;

	bool selfTest = false;

	CANTxFrame frame =
	{
		.DLC	= 1,
		.IDE	= CAN_IDE_STD,
		.SID	= STATUS_MESSAGE_ID,
		.data8	=
		{
			(ltcChain.state != LTC6811_CHAIN_STATE_READY) |
			((overvoltage) << 1) |
			((undervoltage) << 2) |
			((false) << 3) |
			((false) << 4) |
			((senseLine) << 5) |
			((selfTest) << 6)
		}
	};

	return canTransmitTimeout (driver, CAN_ANY_MAILBOX, &frame, timeout);
}

msg_t transmitVoltageMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index)
{
	// TODO(Barach): Remap to in-order.
	uint16_t ltcIndex = index / 2;
	uint8_t voltOffset = (index % 2) * 6;

	uint16_t voltages [6];
	for (uint8_t voltIndex = 0; voltIndex < 6; ++voltIndex)
		voltages [voltIndex] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [voltOffset + voltIndex]);

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
			(voltages [5] >> 6) & 0b1111
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

	for (uint8_t tempIndex = 0; tempIndex < 5; ++tempIndex)
		if (thermistors [index][tempIndex].resistance < 16380.0f)
			temperatures [tempIndex] = thermistors [index][tempIndex].resistance / 4.0f;
		else
			temperatures [tempIndex] = 4095;

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
			(temperatures [4] >> 8) & 0b1111
		}
	};

	return canTransmitTimeout (driver, CAN_ANY_MAILBOX, &frame, timeout);
}

msg_t transmitSenseLineStatusMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index)
{
	// TODO(Barach): Mapping

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