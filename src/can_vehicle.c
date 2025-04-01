// Header
#include "can_vehicle.h"

// TODO(Barach): Temporary
#include "watchdog.h"

// Threads --------------------------------------------------------------------------------------------------------------------

static CAN_THREAD_WORKING_AREA (can1ThreadWa);

// Global Nodes ---------------------------------------------------------------------------------------------------------------

#define NODE_COUNT sizeof (nodes) / sizeof (nodes [0])
static canNode_t* nodes [0];

// Function Prototypes --------------------------------------------------------------------------------------------------------

static int8_t canRxHandler (void* arg, CANRxFrame* frame);

// Configuration --------------------------------------------------------------------------------------------------------------

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

static const canThreadConfig_t CAN1_THREAD_CONFIG =
{
	.name		= "can_1_rx",
	.driver		= &CAND1,
	.period		= TIME_MS2I (1),
	.nodes		= nodes,
	.nodeCount	= NODE_COUNT,
	.rxHandler	= &canRxHandler
};

// Functions ------------------------------------------------------------------------------------------------------------------

bool canVehicleInit (tprio_t priority)
{
	// CAN 1 driver initialization
	if (canStart (&CAND1, &CAN1_CONFIG) != MSG_OK)
		return false;

	// Leave standby mode
	palClearLine (LINE_CAN1_STBY);

	// Initialize the CAN nodes
	// TODO(Barach): Nodes.

	// Create the CAN Thread
	canThreadStart (can1ThreadWa, sizeof (can1ThreadWa), priority, &CAN1_THREAD_CONFIG);
	return true;
}

int8_t canRxHandler (void* arg, CANRxFrame* frame)
{
	// First argument is config
	const canThreadConfig_t* config = (canThreadConfig_t*) arg;

	(void) config;

	// TODO(Barach): Move to write-only command
	if (frame->SID == 0x752)
		watchdogTrigger ();

	return 0;
}