// Header
#include "can_charger.h"

// Includes
#include "can/can_thread.h"
#include "can/receive.h"

// Threads --------------------------------------------------------------------------------------------------------------------

static CAN_THREAD_WORKING_AREA (can1RxThreadWa);

// Global Nodes ---------------------------------------------------------------------------------------------------------------

tcCharger_t charger;

#define NODE_COUNT sizeof (nodes) / sizeof (nodes [0])
static canNode_t* nodes [] =
{
	(canNode_t*) &charger
};

// Configuration --------------------------------------------------------------------------------------------------------------

/**
 * @brief Configuration of the CAN 1 peripheral.
 * @note See section 32.9 of the STM32F405 Reference Manual for more details.
 * @note Also see: https://www.canbusdebugger.com/can-bit-timing-calculator.
 */
static const CANConfig CAN1_CONFIG =
{
	.mcr = 	CAN_MCR_ABOM |		// Automatic bus-off management.
			CAN_MCR_AWUM |		// Automatic wakeup mode.
			CAN_MCR_TXFP,		// Chronologic FIFI priority.
	.btr =	CAN_BTR_SJW (1) |	// Max 2 TQ resynchronization jump.
			CAN_BTR_TS2 (1) |	// 2 TQ for time segment 2
			CAN_BTR_TS1 (10) |	// 11 TQ for time segment 1
			CAN_BTR_BRP (5)		// Baudrate divisor of 6 (500 kbps)
};

static const canThreadConfig_t CAN1_RX_THREAD_CONFIG =
{
	.name			= "can1_rx",
	.driver			= &CAND1,
	.period			= TIME_MS2I (10),
	.nodes			= nodes,
	.nodeCount		= NODE_COUNT,
	.rxHandler		= &receiveBmsMessage,
	.bridgeDriver	= NULL
};

static const tcChargerConfig_t CHARGER_CONFIG =
{
	.driver			= &CAND1,
	.timeoutPeriod	= TIME_MS2I (2000)
};

// Functions ------------------------------------------------------------------------------------------------------------------

bool canChargerInit (tprio_t priority)
{
	// CAN 1 driver initialization
	if (canStart (&CAND1, &CAN1_CONFIG) != MSG_OK)
		return false;

	// Leave standby mode
	palClearLine (LINE_CAN1_STBY);

	// Initialize the CAN nodes
	tcChargerInit (&charger, &CHARGER_CONFIG);

	// Create the CAN RX thread
	canThreadStart (can1RxThreadWa, sizeof (can1RxThreadWa), priority, &CAN1_RX_THREAD_CONFIG);

	return true;
}