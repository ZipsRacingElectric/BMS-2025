#ifndef CAN_VEHICLE_H
#define CAN_VEHICLE_H

// Vehicle CAN Interface ------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.04.01
//
// Description: CAN interface for when the accumulator is in the vehicle. This bus runs at 1 Mbps.

// Includes -------------------------------------------------------------------------------------------------------------------

// ChibiOS
#include "ch.h"

// Functions ------------------------------------------------------------------------------------------------------------------

bool canVehicleInit (tprio_t priority);

#endif // CAN_VEHICLE_H