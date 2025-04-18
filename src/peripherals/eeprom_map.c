// Header
#include "eeprom_map.h"

// Includes
#include "watchdog.h"

// C Standard Library
#include <string.h>

// Constants ------------------------------------------------------------------------------------------------------------------

#define READONLY_COUNT (sizeof (READONLY_ADDRS) / sizeof (READONLY_ADDRS [0]))
static const uint16_t READONLY_ADDRS [] =
{

};

static const void* READONLY_DATA [READONLY_COUNT] =
{

};

static const uint16_t READONLY_SIZES [READONLY_COUNT] =
{

};

// Functions ------------------------------------------------------------------------------------------------------------------

bool eepromReadonlyRead (void* object, uint16_t addr, void* data, uint16_t dataCount)
{
	(void) object;

	for (uint16_t index = 0; index < READONLY_COUNT; ++index)
	{
		if (addr != READONLY_ADDRS [index])
			continue;

		if (dataCount != READONLY_SIZES [index])
			return false;

		memcpy (data, READONLY_DATA [index], dataCount);
		return true;
	}

	return false;
}

bool eepromWriteonlyWrite (void* object, uint16_t addr, const void* data, uint16_t dataCount)
{
	(void) object;

	// TODO(Barach): Nothing uses data yet.
	(void) data;
	(void) dataCount;

	switch (addr)
	{
	case 0x0000:
		watchdogTrigger ();
	}

	return false;
}