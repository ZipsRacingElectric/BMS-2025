// Battery Management System --------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.11.05
//
// Description: Entrypoint and interrupt handlers for the vehicle's battery management system.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_charger.h"
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

	// Start the watchdog timer
	watchdogStart ();

	// Start the appropriate CAN interface. If in the vehicle, use the vehicle interface, otherwise, use the charger interface.
	if (palReadLine (LINE_CHARGER_DETECT))
		canVehicleInit (NORMALPRIO);
	else
		canChargerInit (NORMALPRIO);


	(void) thermistors;

	while (true)
	{
		// Reset the watchdog
		watchdogReset ();

		ltc6811SampleCells (&ltcChain);
		ltc6811SampleGpio (&ltcChain);

		chThdSleepMilliseconds (500);
	}
}