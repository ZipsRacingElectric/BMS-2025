// Header
#include "peripherals.h"

// Global Peripherals ---------------------------------------------------------------------------------------------------------

mc24lc32_t		eeprom;
eepromMap_t*	eepromMap;

ltc6811_t ltcs [LTC_COUNT];
ltc6811DaisyChain_t ltcChain;

thermistorPulldown_t thermistors [LTC_COUNT][LTC6811_GPIO_COUNT];

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
	.timeout		= TIME_MS2I (500),
	.magicString	= EEPROM_MAP_STRING
};

// TODO(Barach): Baudrate should be higher
/// @brief Configuration for the LTC daisy chain.
ltc6811DaisyChainConfig_t ltcChainConfig =
{
	.spiDriver			= &SPID1,
	.spiConfig 			=
	{
		.circular		= false,						// Linear buffer.
		.slave			= false,						// Device is in master mode.
		.cr1			= 0								// 2-line unidirectional, no CRC, MSB first, master mode, clock idles
														// high, data capture on first clock transition.
						| 0b111 << SPI_CR1_BR_Pos,		// Baudrate 328125 bps.
		.cr2			= 0,							// Default CR2 config.
		.data_cb		= NULL,							// No callbacks.
		.error_cb		= NULL,							//
		.ssport			= PAL_PORT (LINE_CS_ISOSPI),	// IsoSPI transceiver CS pin.
		.sspad			= PAL_PAD (LINE_CS_ISOSPI)		//
	},
	.spiMiso			= LINE_SPI1_MISO,
	.devices			= ltcs,
	.deviceCount		= LTC_COUNT,
	.readAttemptCount	= 5,
	.cellAdcMode		= LTC6811_ADC_422HZ, // TODO(Barach): Should be 26Hz
	.gpioAdcMode		= LTC6811_ADC_26HZ,
	.openWireTestIterations	= 4,
	.cellVoltageMax		= 4.2,
	.cellVoltageMin		= 2.7,
	.gpioAdcSensors		=
	{
		{
			(analogSensor_t*) &thermistors [0][0],
			(analogSensor_t*) &thermistors [0][1],
			(analogSensor_t*) &thermistors [0][2],
			(analogSensor_t*) &thermistors [0][3],
			(analogSensor_t*) &thermistors [0][4],
		},
		{
			(analogSensor_t*) &thermistors [1][0],
			(analogSensor_t*) &thermistors [1][1],
			(analogSensor_t*) &thermistors [1][2],
			(analogSensor_t*) &thermistors [1][3],
			(analogSensor_t*) &thermistors [1][4],
		}
	},
};

static const thermistorPulldownConfig_t THERMISTOR_CONFIG =
{
	.steinhartHartA			= 0,
	.steinhartHartB			= 0,
	.steinhartHartC			= 0,
	.steinhartHartD			= 0,
	.resistanceReference	= 10,
	.resistancePullup		= 10000,
	.sampleVdd				= 5000,
	.temperatureMin			= -10,
	.temperatureMax			= 100
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

	// Reconfigurable peripheral initializations. Note this must occur before the LTC initialization as the LTCs are dependent
	// on the thermistor peripherals.
	peripheralsReconfigure ();

	// LTC daisy chain initialization
	ltc6811Init (&ltcChain, &ltcChainConfig);
	return true;
}

void peripheralsReconfigure (void)
{
	// Thermistor initialization
	// TODO(Barach): const discard
	for (uint16_t deviceIndex = 0; deviceIndex < LTC_COUNT; ++deviceIndex)
		for (uint16_t gpioIndex = 0; gpioIndex < LTC6811_GPIO_COUNT; ++gpioIndex)
			thermistorPulldownInit (&thermistors [deviceIndex][gpioIndex], &THERMISTOR_CONFIG);
}