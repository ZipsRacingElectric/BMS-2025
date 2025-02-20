#ifndef LTC6811_H
#define LTC6811_H

// LTC6811-1 Driver -----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.02.17
//
// Description: Device driver for the LTC6811-1 BMS ICs.
//
// Note: This code is derivative of the Analog Devices Linduino codebase:
//   https://github.com/analogdevicesinc/Linduino/tree/master.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "hal.h"

// Constants ------------------------------------------------------------------------------------------------------------------

#define LTC6811_CELL_COUNT		12
#define LTC6811_BUFFER_SIZE		8

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef enum
{
	LTC6811_STATE_PEC_ERROR	= 0,
	LTC6811_STATE_READY		= 1
} ltc6811State_t;

typedef enum
{
	LTC6811_CHAIN_STATE_FAILED	= 0,
	LTC6811_CHAIN_STATE_READY	= 1,
} ltc6811ChainState_t;

typedef struct
{
	ltc6811State_t state;
	float cellVoltages [LTC6811_CELL_COUNT];
	uint8_t tx [LTC6811_BUFFER_SIZE];
	uint8_t rx [LTC6811_BUFFER_SIZE];
} ltc6811_t;

typedef struct
{
	ltc6811ChainState_t state;
	ioline_t miso;
	SPIConfig* config;
	SPIDriver* driver;
	ltc6811_t* devices;
	uint16_t deviceCount;
} ltc6811DaisyChain_t;

// Functions ------------------------------------------------------------------------------------------------------------------

#endif // LTC6811_H