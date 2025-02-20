// Header
#include "peripherals.h"

// Global Peripherals ---------------------------------------------------------------------------------------------------------

mc24lc32_t eeprom;
// eepromMap_t* eepromMap;

// Configuration --------------------------------------------------------------------------------------------------------------

/// @brief Configuration for the I2C1 bus.
static const I2CConfig I2C1_CONFIG =
{
	.op_mode		= OPMODE_I2C,
	.clock_speed	= 400000,
	.duty_cycle		= FAST_DUTY_CYCLE_2
};

/// @brief Configuration for the on-board EEPROM.
static const mc24lc32Config_t EEPROM_CONFIG =
{
	.addr			= 0x50,
	.i2c			= &I2CD1,
	.timeoutPeriod	= TIME_MS2I (500),
	.magicString	= EEPROM_MAP_STRING
};

// Functions ------------------------------------------------------------------------------------------------------------------

void peripheralsInit (void)
{
	// TODO(Barach): Return value

	// I2C 1 driver initialization
	i2cStart (&I2CD1, &I2C1_CONFIG);

	// EEPROM initialization
	mc24lc32Init (&eeprom, &EEPROM_CONFIG);
	eepromMap = (eepromMap_t*) eeprom.cache;

	peripheralsReconfigure ();
}

void peripheralsReconfigure (void)
{
	// TODO(Barach): Nothing to put here yet.
}