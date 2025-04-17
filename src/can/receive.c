// Header
#include "receive.h"

// Includes
#include "can/can_thread.h"
#include "watchdog.h"

int8_t receiveBmsMessage (void* arg, CANRxFrame* frame)
{
	// First argument is config
	const canThreadConfig_t* config = (canThreadConfig_t*) arg;
	(void) config;

	// TODO(Barach): Move to write-only command
	if (frame->SID == 0x752)
		watchdogTrigger ();

	return 0;
}