#ifndef DHAB_S124_H
#define DHAB_S124_H

// DHAB S/124 Current Sensor --------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.04.17
//
// Description: Analog sensor representing the DHAB S/124 current sensor.
//
// TODO(Barach): Figure out all this.

// Includes -------------------------------------------------------------------------------------------------------------------

#include "peripherals/analog_sensor.h"

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	ANALOG_SENSOR_FIELDS;
} dhabS124Channel_t;

typedef struct
{
	dhabS124Channel_t channel1;
	dhabS124Channel_t channel2;
} dhabS124_t;

// Functions ------------------------------------------------------------------------------------------------------------------

void dhabS124Init (dhabS124_t* sensor);

#endif // DHAB_S124_H