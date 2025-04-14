// Vehicle CAN Interface ------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.04.01
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can/can_thread.h"

// ChibiOS
#include "hal.h"

// Global Nodes ---------------------------------------------------------------------------------------------------------------

// TODO(Barach): Nodes.

// Functions ------------------------------------------------------------------------------------------------------------------

bool canVehicleInit (tprio_t priority);

void canVehicleTransmit (sysinterval_t timeout);