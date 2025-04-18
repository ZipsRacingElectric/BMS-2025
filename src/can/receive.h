#ifndef RECEIVE_H
#define RECEIVE_H

// BMS CAN Receivers ----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.04.03
//
// Description: Function for receiving CAN messages that aren't transmitted by a specific CAN node.

// Includes -------------------------------------------------------------------------------------------------------------------

// ChibiOS
#include "hal.h"

// Functions ------------------------------------------------------------------------------------------------------------------

int8_t receiveBmsMessage (void* arg, CANRxFrame* frame);

#endif // RECEIVE_H