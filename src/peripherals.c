// Header
#include "peripherals.h"

// Global Peripherals ---------------------------------------------------------------------------------------------------------

mc24lc32_t		eeprom;
eepromMap_t*	eepromMap;

#define LTC_COUNT 1
ltc6811_t ltcs [LTC_COUNT] =
{
	{
		.state = LTC6811_STATE_READY
	}
};

ltc6811DaisyChain_t	senseBoards =
{
	.state			= LTC6811_CHAIN_STATE_READY,
	.miso			= LINE_SPI1_MISO,
	.config 		=
	{
		.circular	= false,						// Linear buffer.
		.slave		= false,						// Device is in master mode.
		.cr1		= 0								// 2-line unidirectional, no CRC, MSB first, master mode, clock idles high,
													// data capture on first clock transition.
					| 0b111 << SPI_CR1_BR_Pos,		// Baudrate 328125 bps.
		.cr2		= 0,							// Default CR2 config.
		.data_cb	= NULL,							// No callbacks.
		.error_cb	= NULL,							//
		.ssport		= PAL_PORT (LINE_CS_ISOSPI),	// IsoSPI transceiver CS pin.
		.sspad		= PAL_PAD (LINE_CS_ISOSPI)		//
	},
	.driver			= &SPID1,
	.devices		= ltcs,
	.deviceCount	= LTC_COUNT
};

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

bool peripheralsInit (void)
{
	// I2C 1 driver initialization
	if (i2cStart (&I2CD1, &I2C1_CONFIG) != MSG_OK)
		return false;

	// EEPROM initialization
	if (!mc24lc32Init (&eeprom, &EEPROM_CONFIG) && eeprom.state == MC24LC32_STATE_FAILED)
		return false;
	eepromMap = (eepromMap_t*) eeprom.cache;

	// Re-configurable peripherals are not considered fatal.
	peripheralsReconfigure ();
	return true;
}

void peripheralsReconfigure (void)
{
	// TODO(Barach): Nothing to put here yet.
}