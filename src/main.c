// Battery Management System --------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2024.11.05
//
// Description: Entrypoint and interrupt handlers for the vehicle's battery management system.
//
// TODO(Barach):
// - Introduce sense-board object for abstracting thermistor and LTC mapping.
// - Replace LTC init with sequence of 'append' functions to remove unnecessary arrays.
// - Have sense-board take over fault tolerance of LTCs
// - Have LTCs dump cell values into user-provided array so single array can be used.
// - Fix ADC polling

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can_vehicle.h"
#include "can_charger.h"
#include "debug.h"
#include "monitor_thread.h"
#include "peripherals.h"
#include "watchdog.h"

// ChibiOS
#include "hal.h"

// Interrupts -----------------------------------------------------------------------------------------------------------------

void hardFaultCallback (void)
{
	// Open the shutdown loop
	palWriteLine (LINE_BMS_FLT, false);
}

// Entrypoint -----------------------------------------------------------------------------------------------------------------

void vehicleEntrypoint (void);

void chargerEntrypoint (void);

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

	// If detect line is low, accumulator is on charger. Otherwise, accumulator is in vehicle.
	if (palReadLine (LINE_CHARGER_DETECT))
	{
		// Initialize the CAN interface.
		if (!canVehicleInit (NORMALPRIO))
		{
			hardFaultCallback ();
			while (true);
		}

		// Start the monitoring thread.
		monitorThreadStart (NORMALPRIO);

		// Do nothing
		while (true)
			chThdSleepMilliseconds (500);
	}
	else
	{
		// Initialize the CAN interface.
		if (!canChargerInit (NORMALPRIO))
		{
			hardFaultCallback ();
			while (true);
		}

		// Start the monitoring thread.
		monitorThreadStart (NORMALPRIO);

		// Main loop
		systime_t timePrevious = chVTGetSystemTimeX ();
		while (true)
		{
			chMtxLock (&peripheralMutex);

			balancing = hardwareEepromMap->balancingEnabled;
			if (palReadLine (LINE_SHUTDOWN_STATUS) && !bmsFault && balancing)
			{
				// TODO(Barach): Proper fault handling
				float minVoltage = ltcs [0].cellVoltages [0];
				for (uint16_t ltc = 0; ltc < LTC_COUNT; ++ltc)
					for (uint16_t cell = 0; cell < LTC6811_CELL_COUNT; ++cell)
						if (ltcs [ltc].cellVoltages [cell] < minVoltage)
							minVoltage = ltcs [ltc].cellVoltages [cell];

				for (uint16_t ltc = 0; ltc < LTC_COUNT; ++ltc)
					for (uint16_t cell = 0; cell < LTC6811_CELL_COUNT; ++cell)
						ltcs [ltc].cellsDischarging [cell] =
							ltcs [ltc].cellVoltages [cell] - minVoltage > hardwareEepromMap->balancingThreshold;
			}
			else
			{
				for (uint16_t ltc = 0; ltc < LTC_COUNT; ++ltc)
					for (uint16_t cell = 0; cell < LTC6811_CELL_COUNT; ++cell)
						ltcs [ltc].cellsDischarging [cell] = false;
			}

			charging = hardwareEepromMap->chargingEnabled;
			if (palReadLine (LINE_SHUTDOWN_STATUS) && !bmsFault && charging)
			{
				// Calculate the maximum requestable current, based on the power limit.
				float currentLimit = hardwareEepromMap->chargingPowerLimit / packVoltage;

				// Saturate based on the current limit.
				if (currentLimit > hardwareEepromMap->chargingCurrentLimit)
					currentLimit = hardwareEepromMap->chargingCurrentLimit;

				// Send the power request.
				tcChargerSendCommand (&charger, TC_WORKING_MODE_CLOSING,
					hardwareEepromMap->chargingVoltageLimit, currentLimit, TIME_MS2I (100));
			}
			else
			{
				// Disable the charger
				tcChargerSendCommand (&charger, TC_WORKING_MODE_SLEEP, 0, 0, TIME_MS2I (100));
			}

			chMtxUnlock (&peripheralMutex);

			// Sleep until the next loop
			chThdSleepUntilWindowed (timePrevious, chTimeAddX (timePrevious, TIME_MS2I (500)));
			timePrevious = chVTGetSystemTimeX ();
		}
	}
}