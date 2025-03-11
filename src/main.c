// Battery Management System --------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.11.05
//
// Description: Entrypoint and interrupt handlers for the vehicle's battery management system.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "debug.h"
#include "peripherals.h"

// ChibiOS
#include "hal.h"

// Interrupts -----------------------------------------------------------------------------------------------------------------

void hardFaultCallback (void)
{
	// TODO(Barach):
	// Fault handler implementation
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
	if (!peripheralsInit ())
	{
		hardFaultCallback ();
		while (true);
	}

	// Do nothing.
	while (true)
		chThdSleepMilliseconds (500);
}