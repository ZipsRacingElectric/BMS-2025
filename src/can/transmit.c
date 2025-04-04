// Header
#include "transmit.h"

// Conversions -----------------------------------------------------------------------------------------------------------------

// Voltage Values (V)
#define VOLTAGE_INVERSE_FACTOR		(1024.0f / 8.0f)
#define VOLTAGE_TO_WORD(voltage)	(uint8_t) ((voltage) * VOLTAGE_INVERSE_FACTOR)

// Message IDs ----------------------------------------------------------------------------------------------------------------

#define CELL_MESSAGE_BASE_ID	0x700
#define TEMP_MESSAGE_BASE_ID	0x710

// Functions ------------------------------------------------------------------------------------------------------------------

msg_t transmitCellMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index)
{
	uint16_t ltcIndex = index / 2;

	uint16_t voltages[6];

	if (index % 2 == 0)
	{
		voltages [0] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [0]);
		voltages [1] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [1]);
		voltages [2] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [2]);
		voltages [3] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [3]);
		voltages [4] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [4]);
		voltages [5] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [5]);
	}
	else
	{
		voltages [0] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [6]);
		voltages [1] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [7]);
		voltages [2] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [8]);
		voltages [3] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [9]);
		voltages [4] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [10]);
		voltages [5] = VOLTAGE_TO_WORD (ltcs [ltcIndex].cellVoltages [11]);
	}

	CANTxFrame frame =
	{
		.DLC	= 4,
		.IDE	= CAN_IDE_STD,
		.SID	= CELL_MESSAGE_BASE_ID + index,
		.data8 =
		{
			voltages [0],
			(voltages [1] << 2) | ((voltages [0] >> 8) & 0b11),
			(voltages [2] << 4) | ((voltages [1] >> 6) & 0b1111),
			(voltages [3] << 6) | ((voltages [2] >> 4) & 0b111111),
			voltages [3],
			voltages [4],
			(voltages [5] << 2) | ((voltages [4] >> 8) & 0b11),
			(voltages [5] >> 6) & 0b1111
		}
	};

	return canTransmitTimeout (driver, CAN_ANY_MAILBOX, &frame, timeout);
}