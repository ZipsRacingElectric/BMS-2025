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
	if (!peripheralsInit ())
	{
		hardFaultCallback ();
		while (true);
	}

	// TODO(Barach): Check charger pin
	if (!canInterfaceInit (NORMALPRIO))
	{
		hardFaultCallback ();
		while (true);
	}

	// // Start the watchdog timer
	// watchdogStart ();

	while (true)
	{
		// // Reset the watchdog
		// watchdogReset ();

		chMtxLock (&ltcMutex);
		ltc6811ClearState (ltcBottom);
		ltc6811SampleCells (ltcBottom);
		ltc6811SampleGpio (ltcBottom);
		ltc6811OpenWireTest (ltcBottom);
		chMtxUnlock (&ltcMutex);

		chThdSleepMilliseconds (100);
	}
}