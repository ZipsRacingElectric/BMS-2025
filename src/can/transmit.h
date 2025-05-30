#ifndef TRANSMIT_H
#define TRANSMIT_H

// BMS CAN Transmitters -------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.04.03
//
// Description: Functions for transmitting CAN messages that aren't directed towards a specific CAN node.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "peripherals.h"

// Constants ------------------------------------------------------------------------------------------------------------------

#define VOLTAGE_MESSAGE_COUNT ((CELL_COUNT + 5) / 6)
#define TEMPERATURE_MESSAGE_COUNT ((TEMP_COUNT + 4) / 5)
#define SENSE_LINE_STATUS_MESSAGE_COUNT ((WIRE_COUNT + 51) / 52)
#define BALANCING_MESSAGE_COUNT ((CELL_COUNT + 47) / 48)

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Transmits all of the BMS's regular CAN messages: status, power, cell voltages, temperatures, and sense-line statuses.
 * @param timeout The interval to timeout after.
 */
void transmitBmsMessages (sysinterval_t timeout);

/**
 * @brief Transmits the BMS status message based on the current fault conditions.
 * @param driver The CAN driver to use.
 * @param timeout The interval to timeout after.
 * @return The result of the CAN operation.
 */
msg_t transmitStatusMessage (CANDriver* driver, sysinterval_t timeout);

/**
 * @brief Transmits the BMS power consumption message.
 * @param driver The CAN driver to use.
 * @param timeout The interval to timeout after.
 * @return The result of the CAN operation.
 */
msg_t transmitPowerMessage (CANDriver* driver, sysinterval_t timeout);

/**
 * @brief Transmits a cell voltage message based on the current cell voltages.
 * @param driver The CAN driver to use.
 * @param timeout The interval to timeout after.
 * @param index The index of the message to send.
 * @return The result of the CAN operation.
 */
msg_t transmitVoltageMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index);

/**
 * @brief Transmits a sense-line temperature message
 * @param driver The CAN driver to use.
 * @param timeout The interval to timeout after.
 * @param index The index of the message to send.
 * @return The result of the CAN operation.
 */
msg_t transmitTemperatureMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index);

/**
 * @brief Transmits a sense-line status message
 * @param driver The CAN driver to use.
 * @param timeout The interval to timeout after.
 * @param index The index of the message to send.
 * @return The result of the CAN operation.
 */
msg_t transmitSenseLineStatusMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index);

/**
 * @brief Transmits a cell balancing message
 * @param driver The CAN driver to use.
 * @param timeout The interval to timeout after.
 * @param index The index of the message to send.
 * @return The result of the CAN operation.
 */
msg_t transmitBalancingMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index);

#endif // TRANSMIT_H