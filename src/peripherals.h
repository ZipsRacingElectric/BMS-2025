#ifndef PERIPHERALS_H
#define PERIPHERALS_H

// Peripherals ----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.02.20
//
// Description: Global objects representing the grounded low-voltage hardware of the BMS.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "peripherals/eeprom_map.h"
#include "peripherals/ltc6811.h"
#include "peripherals/mc24lc32.h"

// Global Peripherals ---------------------------------------------------------------------------------------------------------

/// @brief The BMS's EEPROM. This is responsible for storing all non-volatile variables.
extern mc24lc32_t eeprom;

/// @brief Structure mapping the EEPROM's contents to C datatypes.
extern eepromMap_t* eepromMap;

// TODO(Barach): Docs
extern ltc6811DaisyChain_t ltcChain;

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the BMS's peripherals.
 * @return False if a fatal peripheral failed to initialize, true otherwise.
 */
bool peripheralsInit (void);

/**
 * @brief Re-initializes the BMS's peripherals after a change has been made to the on-board EEPROM.
 */
void peripheralsReconfigure (void);

#endif // PERIPHERALS_H