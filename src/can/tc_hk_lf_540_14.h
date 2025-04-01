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
	TC_CHARGER_INPUT_VOLTAGE_NORMAL		= 0b00,
	TC_CHARGER_INPUT_VOLTAGE_UNDER_VOLT	= 0b01,
	TC_CHARGER_INPUT_VOLTAGE_OVER_VOLT	= 0b10,
	TC_CHARGER_INPUT_VOLTAGE_NO_INPUT	= 0b11
} tcChargerInputVoltageStatus_t;

typedef struct
{
	CANDriver* driver;
	sysinterval_t timeoutPeriod;
} tcChargerConfig_t;

typedef struct
{
	CAN_NODE_FIELDS;

	float outputVoltage;
	float outputCurrent;
	bool hardwareProtection;
	bool temperatureProtection;
	tcChargerInputVoltageStatus_t inputVoltageStatus;
} tcCharger_t;

// Functions ------------------------------------------------------------------------------------------------------------------

void tcChargerInit (tcCharger_t* charger, tcChargerConfig_t* config);

#endif // TC_HK_LF_540_14_H