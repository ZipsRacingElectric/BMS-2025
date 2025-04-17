#ifndef CAN_CHARGER_H
#define CAN_CHARGER_H

// Charger CAN Interface ------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.04.16
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can/tc_hk_lf_540_14.h"

// Global Nodes ---------------------------------------------------------------------------------------------------------------

/// @brief The TC on-board charger.
extern tcCharger_t charger;

// Functions ------------------------------------------------------------------------------------------------------------------

bool canChargerInit (tprio_t priority);

#endif // CAN_CHARGER_H