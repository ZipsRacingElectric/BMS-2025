// Header
#include "tc_hk_lf_540_14.h"

// Conversions ----------------------------------------------------------------------------------------------------------------

// Voltage
#define VOLTAGE_FACTOR			0.1f
#define VOLTAGE_INVERSE_FACTOR	10.0f
#define WORD_TO_VOLTAGE(word)	((word) * VOLTAGE_FACTOR)
#define VOLTAGE_TO_WORD(volt)	((uint16_t) ((volt) * VOLTAGE_INVERSE_FACTOR))

// Current
#define CURRENT_FACTOR			0.1f
#define CURRENT_INVERSE_FACTOR	10.0f
#define WORD_TO_CURRENT(word)	((word) * CURRENT_FACTOR)
#define CURRENT_TO_WORD(curr)	((uint16_t) ((curr) * CURRENT_INVERSE_FACTOR))

// Status Word
#define STATUS_WORD_TO_HARDWARE_PROTECTION(word)		(((word) & 0b000000000000000000000001) == 0b000000000000000000000001)
#define STATUS_WORD_TO_TEMPERATURE_PROTECTION(word)		(((word) & 0b000000000000000000000010) == 0b000000000000000000000010)
#define STATUS_WORD_TO_INPUT_VOLTAGE_STATUS(word)		(((word) & 0b000000000000000000001100) >> 2)
#define STATUS_WORD_TO_OUTPUT_UNDERVOLTAGE(word)		(((word) & 0b000000000000000000010000) == 0b000000000000000000010000)
#define STATUS_WORD_TO_OUTPUT_OVERVOLTAGE(word)			(((word) & 0b000000000000000000100000) == 0b000000000000000000100000)
#define STATUS_WORD_TO_OUTPUT_OVERCURRENT(word)			(((word) & 0b000000000000000001000000) == 0b000000000000000001000000)
#define STATUS_WORD_TO_OUTPUT_SHORT_CIRCUIT(word)		(((word) & 0b000000000000000010000000) == 0b000000000000000010000000)
#define STATUS_WORD_TO_COMMAND_FAULT(word)				(((word) & 0b000000000000000100000000) == 0b000000000000000100000000)
#define STATUS_WORD_TO_WORK_STATUS(word)				(((word) & 0b000000000000011000000000) >> 9)
#define STATUS_WORD_TO_INITIALIZED(word)				(((word) & 0b000000000000100000000000) == 0b000000000000100000000000)
#define STATUS_WORD_TO_SOCKET_OVERTEMPERATURE(word)		(((word) & 0b000010000000000000000000) == 0b000010000000000000000000)

#define STATUS_WORD_INPUT_VOLTAGE_STATUS_NORMAL			0
#define STATUS_WORD_WORK_STATUS_WORKING					1
#define STATUS_WORD_WORK_STATUS_STOPPED					2
#define STATUS_WORD_WORK_STATUS_STANDBY					3

// Message IDs ----------------------------------------------------------------------------------------------------------------

#define RESPONSE_ID 0x3E5

// Function Prototypes --------------------------------------------------------------------------------------------------------

int8_t tcChargerReceiveHandler (void* node, CANRxFrame* frame);

// Functions ------------------------------------------------------------------------------------------------------------------

void tcChargerInit (tcCharger_t* charger, const tcChargerConfig_t* config)
{
	// Initialize the node
	canNodeConfig_t canConfig =
	{
		.driver			= config->driver,
		.receiveHandler	= tcChargerReceiveHandler,
		.timeoutHandler	= NULL,
		.timeoutPeriod	= config->timeoutPeriod,
		.messageCount	= 1
	};
	canNodeInit ((canNode_t*) charger, &canConfig);
}

int8_t tcChargerReceiveHandler (void* node, CANRxFrame* frame)
{
	// Charger only has 1 message.
	if (frame->SID != RESPONSE_ID)
		return -1;

	tcCharger_t* charger = (tcCharger_t*) node;

	charger->outputVoltage = WORD_TO_VOLTAGE (frame->data16 [0]);
	charger->outputCurrent = WORD_TO_CURRENT (frame->data16 [1]);

	uint32_t statusWord = frame->data32 [1] & 0xFFFFFF;

	// Fault conditions
	bool faulted =
		(STATUS_WORD_TO_INPUT_VOLTAGE_STATUS (statusWord) != STATUS_WORD_INPUT_VOLTAGE_STATUS_NORMAL) |
		STATUS_WORD_TO_HARDWARE_PROTECTION (statusWord) |
		STATUS_WORD_TO_TEMPERATURE_PROTECTION (statusWord) |
		STATUS_WORD_TO_OUTPUT_UNDERVOLTAGE (statusWord) |
		STATUS_WORD_TO_OUTPUT_OVERVOLTAGE (statusWord) |
		STATUS_WORD_TO_OUTPUT_OVERCURRENT (statusWord) |
		STATUS_WORD_TO_OUTPUT_SHORT_CIRCUIT (statusWord) |
		STATUS_WORD_TO_COMMAND_FAULT (statusWord) |
		STATUS_WORD_TO_INITIALIZED (statusWord) |
		STATUS_WORD_TO_SOCKET_OVERTEMPERATURE (statusWord);

	if (!faulted)
	{
		// Non-faulted, check work status for charging state
		switch (STATUS_WORD_TO_WORK_STATUS (statusWord))
		{
		case STATUS_WORD_WORK_STATUS_WORKING:
			charger->chargingState = TC_CHARGER_CHARGING;
			break;
		case STATUS_WORD_WORK_STATUS_STOPPED:
		case STATUS_WORD_WORK_STATUS_STANDBY:
			charger->chargingState = TC_CHARGER_IDLE;
			break;
		default:
			charger->chargingState = TC_CHARGER_FAULTED;
		}
	}
	else
	{
		// Faulted
		charger->chargingState = TC_CHARGER_FAULTED;
	}

	return 0;
}