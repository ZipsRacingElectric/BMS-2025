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
#include "can_charger.h"
#include "can/transmit.h"

// ChibiOS
#include "hal.h"

// Constants ------------------------------------------------------------------------------------------------------------------

#define BMS_THREAD_PERIOD TIME_MS2I (250)

// Interrupts -----------------------------------------------------------------------------------------------------------------

void hardFaultCallback (void)
{
	// Open the shutdown loop
	palWriteLine (LINE_BMS_FLT, false);
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

	// // Start the watchdog timer.
	// watchdogStart ();

	// TODO(Barach): Move most of this logic into peripherals, or is that too far?

	// If detect line is low, accumulator is on charger. Otherwise, accumulator is in vehicle.
	if (palReadLine (LINE_CHARGER_DETECT))
	{
		// Vehicle mode

		// Initialize the CAN interface.
		if (!canVehicleInit (NORMALPRIO))
		{
			hardFaultCallback ();
			while (true);
		}

		// Main loop
		systime_t timePrevious = chVTGetSystemTimeX ();
		while (true)
		{
			// // Reset the watchdog.
			// watchdogReset ();

			// Sample the LTCs.
			peripheralsSample ();

			// If a fault is present, open the shutdown loop.
			bool fltLine = !bmsFault;
			palWriteLine (LINE_BMS_FLT, fltLine);

			// Transmit the CAN messages.
			transmitBmsMessages (BMS_THREAD_PERIOD);

			// Sleep until the next loop
			chThdSleepUntilWindowed (timePrevious, chTimeAddX (timePrevious, BMS_THREAD_PERIOD));
			timePrevious = chVTGetSystemTimeX ();
		}
	}
	else
	{
		// Charger mode

		// Initialize the CAN interface.
		if (!canChargerInit (NORMALPRIO))
		{
			hardFaultCallback ();
			while (true);
		}

		// Main loop
		systime_t timePrevious = chVTGetSystemTimeX ();
		while (true)
		{
			// // Reset the watchdog.
			// watchdogReset ();

			// Sample the LTCs.
			peripheralsSample ();

			// Balance the cells
			// TODO(Barach): Move to peripherals?
			// TODO(Barach): Different thread or just different period?
			// TODO(Barach): Conditional balancing (make config public variable)

			float minVoltage = ltcs [0].cellVoltages [0];
			for (uint16_t ltc = 0; ltc < LTC_COUNT; ++ltc)
				for (uint16_t cell = 0; cell < LTC6811_CELL_COUNT; ++cell)
					if (ltcs [ltc].cellVoltages [cell] < minVoltage)
						minVoltage = ltcs [ltc].cellVoltages [cell];

			for (uint16_t ltc = 0; ltc < LTC_COUNT; ++ltc)
				for (uint16_t cell = 0; cell < LTC6811_CELL_COUNT; ++cell)
					ltcs [ltc].cellsDischarging [cell] = ltcs [ltc].cellVoltages [cell] - minVoltage > 0.5f;

			ltc6811WriteConfig (ltcBottom);

			// If a fault is present, open the shutdown loop.
			bool fltLine = !bmsFault;
			palWriteLine (LINE_BMS_FLT, fltLine);

			// Transmit the CAN messages.
			transmitBmsMessages (BMS_THREAD_PERIOD);

			// Sleep until the next loop
			chThdSleepUntilWindowed (timePrevious, chTimeAddX (timePrevious, BMS_THREAD_PERIOD));
			timePrevious = chVTGetSystemTimeX ();
		}
	}
}