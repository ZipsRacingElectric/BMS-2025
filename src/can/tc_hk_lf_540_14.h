#ifndef TC_HK_LF_540_14_H
#define TC_HK_LF_540_14_H

// TC Charger Model HK-LF-540-14 CAN Interface --------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.02.21
//
// Description: CAN node representing the TC 6.6kW on-board charger.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "can/can_node.h"

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	TC_CHARGER_FAULTED	= 0,
	TC_CHARGER_IDLE		= 1,
	TC_CHARGER_CHARGING	= 2
} tcChargingState_t;

typedef struct
{
	CANDriver* driver;
	sysinterval_t timeoutPeriod;
} tcChargerConfig_t;

typedef struct
{
	CAN_NODE_FIELDS;

	const tcChargerConfig_t* config;

	tcChargingState_t chargingState;
	float outputVoltage;
	float outputCurrent;
} tcCharger_t;

// Functions ------------------------------------------------------------------------------------------------------------------

void tcChargerInit (tcCharger_t* charger, const tcChargerConfig_t* config);

#endif // TC_HK_LF_540_14_H