// Charger CAN Interface ------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.04.01
//
// Description: Thread for managing the BMS's CAN interface while on the charger.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can/can_thread.h"

// ChibiOS
#include "hal.h"

// Global Nodes ---------------------------------------------------------------------------------------------------------------

// TODO(Barach): Nodes.

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Creates and starts the CAN interface thread.
 * @param priority The priority of the thread.
 * @return False if a fatal error occurred, true otherwise.
 */
bool canChargerInit (tprio_t priority);