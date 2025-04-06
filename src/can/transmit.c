// Header
#include "transmit.h"

// Conversions -----------------------------------------------------------------------------------------------------------------

// Voltage Values (V)
#define VOLTAGE_INVERSE_FACTOR		(1024.0f / 8.0f)
#define VOLTAGE_TO_WORD(voltage)	(uint16_t) ((voltage) * VOLTAGE_INVERSE_FACTOR)

// Message IDs ----------------------------------------------------------------------------------------------------------------

#define CELL_MESSAGE_BASE_ID		0x700
#define TEMP_MESSAGE_BASE_ID		0x710
#define SENSE_LINE_STATUS_BASE_ID	0x720

// Functions ------------------------------------------------------------------------------------------------------------------

msg_t transmitVoltageMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index)
{
	// TODO(Barach): Remap to in-order.
	uint16_t ltcIndex = index / 2;
	uint8_t voltageOffset = (index % 2) * 6;

	uint16_t voltages[6];

	voltages [0] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [voltageOffset]);
	voltages [1] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [voltageOffset + 1]);
	voltages [2] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [voltageOffset + 2]);
	voltages [3] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [voltageOffset + 3]);
	voltages [4] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [voltageOffset + 4]);
	voltages [5] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [voltageOffset + 5]);

	CANTxFrame frame =
	{
		.DLC	= 8,
		.IDE	= CAN_IDE_STD,
		.SID	= CELL_MESSAGE_BASE_ID + index,
		.data8 =
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
	// TODO(Barach): Mapping

	// if (index % 2 == 0)
	// {
	// 	// Low sense board
	// }
	return MSG_OK;
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