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

// Message IDs ----------------------------------------------------------------------------------------------------------------

#define FEEDBACK_ID 0x3E5

// Function Prototypes --------------------------------------------------------------------------------------------------------

int8_t tcChargerReceiveHandler (void* node, CANRxFrame* frame);

// Functions ------------------------------------------------------------------------------------------------------------------

void tcChargerInit (tcCharger_t* charger, tcChargerConfig_t* config)
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
	if (frame->SID != FEEDBACK_ID)
		return -1;
}