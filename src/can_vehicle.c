// Header
#include "can_vehicle.h"

#include "can/transmit.h"
#include "can/receive.h"

// Threads --------------------------------------------------------------------------------------------------------------------

static CAN_THREAD_WORKING_AREA (can1RxThreadWa);

static THD_WORKING_AREA (can1TxThreadWa, 512);

// Global Nodes ---------------------------------------------------------------------------------------------------------------

#define NODE_COUNT sizeof (nodes) / sizeof (nodes [0])
static canNode_t* nodes [0];

// Function Prototypes --------------------------------------------------------------------------------------------------------

static void can1TxThread (void* arg);

// Configuration --------------------------------------------------------------------------------------------------------------

#define TX_THREAD_PERIOD TIME_MS2I(200)

/**
 * @brief Configuration of the CAN 1 peripheral.
 * @note See section 32.9 of the STM32F405 Reference Manual for more details.
 */
static const CANConfig CAN1_CONFIG =
{
	.mcr = 	CAN_MCR_ABOM |		// Automatic bus-off management.
			CAN_MCR_AWUM |		// Automatic wakeup mode.
			CAN_MCR_TXFP,		// Chronologic FIFI priority.
	.btr =	CAN_BTR_SJW (0) |	// Max 1 TQ resynchronization jump.
			CAN_BTR_TS2 (1) |	// 2 TQ for time segment 2
			CAN_BTR_TS1 (10) |	// 11 TQ for time segment 1
			CAN_BTR_BRP (2)		// Baudrate divisor of 3 (1 Mbps)
};

static const canThreadConfig_t CAN1_RX_THREAD_CONFIG =
{
	.name		= "can_1_rx",
	.driver		= &CAND1,
	.period		= TIME_MS2I (10),
	.nodes		= nodes,
	.nodeCount	= NODE_COUNT,
	.rxHandler	= &can1RxHandler
};

// Functions ------------------------------------------------------------------------------------------------------------------

bool canInterfaceInit (tprio_t priority)
{
	// CAN 1 driver initialization
	if (canStart (&CAND1, &CAN1_CONFIG) != MSG_OK)
		return false;

	// Leave standby mode
	palClearLine (LINE_CAN1_STBY);

	// Initialize the CAN nodes
	// TODO(Barach): Nodes.

	// Create the CAN RX thread
	canThreadStart (can1RxThreadWa, sizeof (can1RxThreadWa), priority, &CAN1_RX_THREAD_CONFIG);

	// Create the CAN TX thread
	chThdCreateStatic (can1TxThreadWa, sizeof (can1TxThreadWa), priority, can1TxThread, NULL);

	return true;
}

void can1TxThread (void* arg)
{
	(void) arg;

	chRegSetThreadName ("can_1_tx");

	while (true)
	{
		for (uint16_t index = 0; index < VOLTAGE_MESSAGE_COUNT; ++index)
			transmitVoltageMessage (&CAND1, TX_THREAD_PERIOD, index);

		for (uint16_t index = 0; index < SENSE_LINE_STATUS_MESSAGE_COUNT; ++index)
			transmitSenseLineStatusMessage (&CAND1, TX_THREAD_PERIOD, index);

		chThdSleep (TX_THREAD_PERIOD);
	}
}