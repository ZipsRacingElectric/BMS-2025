// Watchdog Timer -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.03.15
//
// Description: Global watchdog for the BMS. Used to detect stalled / crashed code and shutdown the vehicle. After starting the
//   watchdog, reset function must be called at least once per second to prevent shutdown.

// Includes -------------------------------------------------------------------------------------------------------------------

// ChibiOS
#include "hal.h"

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Starts the watchdog timer. If the system was previously reset by the watchdog, the system will be halted and
 * shutdown.
 */
void watchdogStart (void);

/**
 * @brief Triggers a shutdown by the watchdog timer.
 */
void watchdogTrigger (void);

/**
 * @brief Resets the watchdog timer to prevent shutdown. Must be called at least once per second.
 */
void watchdogReset (void);