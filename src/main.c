// Battery Management System --------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.11.05
//
// Description: Entrypoint and interrupt handlers for the vehicle's battery management system.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_vehicle.h"
#include "debug.h"
#include "peripherals.h"
#include "watchdog.h"

// ChibiOS
#include "hal.h"

// Interrupts -----------------------------------------------------------------------------------------------------------------

void hardFaultCallback (void)
{
	// TODO(Barach):
	// Fault handler implementation

	palWriteLine (LINE_SHUTDOWN_STATUS, false);
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (void)
{
	// ChibiOS Initialization
	halInit ();
	chSysInit ();

	// Debug Initialization
	ioline_t ledLine = LINE_LED_HEARTBEAT;
	debugHeartbeatStart (&ledLine, LOWPRIO);

	// Peripheral Initialization
	// if (!peripheralsInit ())
	// {
	// 	hardFaultCallback ();
	// 	while (true);
	// }

	ltcs [0].cellVoltages [0] = 0.0f;
	ltcs [0].cellVoltages [1] = 0.1f;
	ltcs [0].cellVoltages [2] = 0.2f;
	ltcs [0].cellVoltages [3] = 0.3f;
	ltcs [0].cellVoltages [4] = 0.4f;
	ltcs [0].cellVoltages [5] = 0.5f;
	ltcs [0].cellVoltages [6] = 0.6f;
	ltcs [0].cellVoltages [7] = 0.7f;
	ltcs [0].cellVoltages [8] = 0.8f;
	ltcs [0].cellVoltages [9] = 0.9f;
	ltcs [0].cellVoltages [10] = 1.0f;
	ltcs [0].cellVoltages [11] = 1.1f;

	if (!canInterfaceInit (NORMALPRIO))
	{
		hardFaultCallback ();
		while (true);
	}

	// Start the watchdog timer
	watchdogStart ();

	(void) thermistors;

	while (true)
	{
		// Reset the watchdog
		watchdogReset ();

		ltc6811SampleCells (&ltcChain);
		ltc6811SampleGpio (&ltcChain);
		ltc6811OpenWireTest (&ltcChain);

		chThdSleepMilliseconds (500);
	}
}