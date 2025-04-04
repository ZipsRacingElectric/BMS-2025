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

#define CELL_MESSAGE_COUNT ((CELL_COUNT + 5) / 6)
#define TEMP_MESSAGE_COUNT ((TEMP_COUNT + 7) / 8)

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Transmits a cell voltage message based on the current cell voltages.
 * @param driver The CAN driver to use.
 * @param timeout The interval to timeout after.
 * @param index The index of the message to send.
 * @return The result of the CAN operation.
 */
msg_t transmitCellMessage (CANDriver* driver, sysinterval_t timeout, uint16_t index);

#endif // TRANSMIT_H