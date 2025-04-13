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
#include "watchdog.h"
#include "can_vehicle.h"
#include "can/transmit.h"

// ChibiOS
#include "hal.h"

// Constants ------------------------------------------------------------------------------------------------------------------

#define BMS_THREAD_PERIOD TIME_MS2I (500)

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

	systime_t timePrevious = chVTGetSystemTimeX ();
	while (true)
	{
		// // Reset the watchdog
		// watchdogReset ();

		// Sample the LTCs
		ltc6811ClearState (ltcBottom);
		ltc6811SampleCells (ltcBottom);
		ltc6811SampleGpio (ltcBottom);
		ltc6811OpenWireTest (ltcBottom);

		// Status message
		transmitStatusMessage (&CAND1, BMS_THREAD_PERIOD);

		// Cell voltage messages
		for (uint16_t index = 0; index < VOLTAGE_MESSAGE_COUNT; ++index)
			transmitVoltageMessage (&CAND1, BMS_THREAD_PERIOD, index);

		// Sense line temperature messages
		for (uint16_t index = 0; index < TEMPERATURE_MESSAGE_COUNT; ++index)
			transmitTemperatureMessage (&CAND1, BMS_THREAD_PERIOD, index);

		// Sense line status messages
		for (uint16_t index = 0; index < SENSE_LINE_STATUS_MESSAGE_COUNT; ++index)
			transmitSenseLineStatusMessage (&CAND1, BMS_THREAD_PERIOD, index);

		chThdSleepUntilWindowed (timePrevious, chTimeAddX (timePrevious, BMS_THREAD_PERIOD));
		timePrevious = chVTGetSystemTimeX ();
	}
}