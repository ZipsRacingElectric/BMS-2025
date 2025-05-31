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
#include "peripherals/dhab_s124.h"
#include "peripherals/eeprom_map.h"
#include "peripherals/ltc6811.h"
#include "peripherals/mc24lc32.h"
#include "peripherals/stm_adc.h"
#include "peripherals/thermistor_pulldown.h"

// Constants ------------------------------------------------------------------------------------------------------------------

/// @brief The number of LTC BMS ICs in the daisy chain. Note this must be even.
#define LTC_COUNT 2

/// @brief The number of cells in the accumulator.
#define CELL_COUNT (LTC_COUNT * LTC6811_CELL_COUNT)

/// @brief The number of sense lines in the accumulator.
#define WIRE_COUNT (LTC_COUNT * (LTC6811_CELL_COUNT + 1))

/// @brief The number of temperature sensors in the accumulator.
#define TEMP_COUNT (LTC_COUNT * LTC6811_GPIO_COUNT)

// Global State ---------------------------------------------------------------------------------------------------------------

/// @brief The voltage of the entire pack, as measured by the LTCs.
extern float packVoltage;

/// @brief Indicates whether any faults are present.
extern bool bmsFault;

/// @brief Indicates an undervoltage fault is present.
extern bool undervoltageFault;

/// @brief Indicates an overvoltage fault is present.
extern bool overvoltageFault;

/// @brief Indicates an undertemperature fault is present.
extern bool undertemperatureFault;

/// @brief Indicates an overtemperature fault is present.
extern bool overtemperatureFault;

/// @brief Indicates a sense-line fault is present.
extern bool senseLineFault;

/// @brief Indicates an IsoSPI fault is present.
extern bool isospiFault;

/// @brief Indicates an LTC self-test fault is present.
extern bool selfTestFault;

/// @brief Indicates the BMS is in charging mode and the charger is powered.
extern bool charging;

/// @brief Indicates the BMS is balancing cell voltages.
extern bool balancing;

/// @brief Indicates the shutdown loop is closed (up to precharge circuit).
extern bool shutdownLoopClosed;

/// @brief Indicates the shutdown loop is closed and precharge is complete.
extern bool prechargeComplete;

// Global Peripherals ---------------------------------------------------------------------------------------------------------

/// @brief Mutex guarding access to the global peripherals.
extern mutex_t peripheralMutex;

/// @brief The STM's on-board ADC.
extern stmAdc_t adc;

/// @brief The BMS's hardware (on-board) EEPROM. This is responsible for storing all non-volatile variables.
extern mc24lc32_t hardwareEeprom;

/// @brief Structure mapping the hardware EEPROM's contents to C datatypes.
extern eepromMap_t* hardwareEepromMap;

/// @brief The BMS's virtual memory map. This aggregates all non-volatile memory into a single map for CAN-bus access.
extern virtualEeprom_t virtualEeprom;

/// @brief The BMS's sense-board ICs. Indexed from negative-most potential LTC to positive-most potential LTC.
extern ltc6811_t ltcs [LTC_COUNT];

/// @brief The first LTC in the IsoSPI daisy chain. Used as the operand in all LTC operations.
extern ltc6811_t* ltcBottom;

/// @brief The BMS's sense-board thermistors. Indexed from negative-most potential to positive-most potential, then by GPIO
/// index.
extern thermistorPulldown_t thermistors [LTC_COUNT][LTC6811_GPIO_COUNT];

/// @brief The BMS's pack current sensor.
extern dhabS124_t currentSensor;

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