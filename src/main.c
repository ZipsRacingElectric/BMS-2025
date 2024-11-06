// Battery Management System --------------------------------------------------------------------------------------------------
// 
// Author: Cole Barach
// Date Created: 2024.11.05

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "debug.h"

// ChibiOS
#include "ch.h"
#include "hal.h"

// Entrypoint -----------------------------------------------------------------------------------------------------------------

int main (void)
{
	// ChibiOS Initialization
	halInit ();
	chSysInit ();

	// Debug Initialization
	debugInit ("Battery Management Board, Revision AA");

	// Do nothing.
	while (true)
		chThdSleepMilliseconds (500);
}

void faultCallback (void)
{
	// TODO(Barach):
	// Fault handler implementation
}