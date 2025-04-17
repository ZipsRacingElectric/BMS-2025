#ifndef PERIPHERALS_H
#define PERIPHERALS_H

// Peripherals ----------------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.02.20
//
// Description: Global objects representing the grounded low-voltage hardware of the BMS.
//
// TODO(Barach): Move configs to EEPROM.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "peripherals/analog_linear.h"
#include "peripherals/eeprom_map.h"
#include "peripherals/ltc6811.h"
#include "peripherals/mc24lc32.h"
#include "peripherals/stm_adc.h"
#include "peripherals/thermistor_pulldown.h"

// Constants ------------------------------------------------------------------------------------------------------------------

#define LTC_COUNT 2
#define CELL_COUNT (LTC_COUNT * LTC6811_CELL_COUNT)
#define TEMP_COUNT (LTC_COUNT * LTC6811_GPIO_COUNT)

// Global State ---------------------------------------------------------------------------------------------------------------

extern float packVoltage;

extern bool bmsFault;
extern bool undervoltageFault;
extern bool overvoltageFault;
extern bool undertemperatureFault;
extern bool overtemperatureFault;
extern bool senseLineFault;
extern bool isoSpiFault;
extern bool selfTestFault;

// Global Peripherals ---------------------------------------------------------------------------------------------------------

/// @brief The BMS's EEPROM. This is responsible for storing all non-volatile variables.
extern mc24lc32_t eeprom;

/// @brief Structure mapping the EEPROM's contents to C datatypes.
extern eepromMap_t* eepromMap;

// TODO(Barach): Docs
extern ltc6811_t ltcs [LTC_COUNT];
extern ltc6811_t* ltcBottom;

// TODO(Barach): Docs
extern thermistorPulldown_t thermistors [LTC_COUNT][LTC6811_GPIO_COUNT];

/// @brief The STM's on-board ADC, used for sampling the current sensor.
extern stmAdc_t adc;

/// @brief The accumulator's pack current sensor.
extern linearSensor_t currentSensor;

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

/**
 * @brief Samples the cell voltages, temperatures, and statuses of all the LTCs within the BMS.
 */
void peripheralsSample (void);

#endif // PERIPHERALS_H