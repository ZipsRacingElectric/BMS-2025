#ifndef EEPROM_MAP_H
#define EEPROM_MAP_H

// EEPROM Mapping -------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.02.20
//
// Description: Structing mapping the data of an EEPROM data to variables.

// Includes -------------------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdint.h>

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief The magic string of the EEPROM. Update this value every time the memory map changes to force manual re-programming.
#define EEPROM_MAP_STRING "BMS_2025_02_20"

// Datatypes ------------------------------------------------------------------------------------------------------------------

typedef struct
{
	uint8_t pad0 [16]; // 0x0000
	// TODO(Barach): Nothing else yet.
} eepromMap_t;

// TODO(Barach): No readonly data yet.

#endif // EEPROM_MAP_H