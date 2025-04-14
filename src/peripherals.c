// Header
#include "peripherals.h"

// Global State ---------------------------------------------------------------------------------------------------------------

float packVoltage = 0.0f;

bool bmsFault = true;
bool undervoltageFault = true;
bool overvoltageFault = true;
bool undertemperatureFault = true;
bool overtemperatureFault = true;
bool senseLineFault = true;
bool isoSpiFault = true;
bool selfTestFault = true;

// Global Peripherals ---------------------------------------------------------------------------------------------------------

mc24lc32_t		eeprom;
eepromMap_t*	eepromMap;

ltc6811_t		ltcs [LTC_COUNT];
ltc6811_t*		ltcBottom;

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

/// @brief Configuration for the LTC daisy chain.
static const ltc6811Config_t DAISY_CHAIN_CONFIG =
{
	.spiDriver				= &SPID1,
	.spiConfig 				=
	{
		.circular			= false,						// Linear buffer.
		.slave				= false,						// Device is in master mode.
		.cr1				= 0								// 2-line unidirectional, no CRC, MSB first, master mode, clock
															// idles high, data capture on first clock transition.
							| 0b110 << SPI_CR1_BR_Pos,		// Baudrate 656250 bps.
		.cr2				= 0,							// Default CR2 config.
		.data_cb			= NULL,							// No callbacks.
		.error_cb			= NULL,							//
		.ssport				= PAL_PORT (LINE_CS_ISOSPI),	// IsoSPI transceiver CS pin.
		.sspad				= PAL_PAD (LINE_CS_ISOSPI)		//
	},
	.spiMiso				= LINE_SPI1_MISO,
	.readAttemptCount		= 5,
	.cellAdcMode			= LTC6811_ADC_422HZ, // TODO(Barach): Should be 26Hz
	.gpioAdcMode			= LTC6811_ADC_26HZ,
	.openWireTestIterations	= 3,
	.faultCount				= 6,
	.cellVoltageMax			= 4.1,
	.cellVoltageMin			= 2.7,
	.gpioSensors =
	{
		{
			(analogSensor_t*) &thermistors [1][0],
			(analogSensor_t*) &thermistors [1][1],
			(analogSensor_t*) &thermistors [1][2],
			(analogSensor_t*) &thermistors [1][3],
			(analogSensor_t*) &thermistors [1][4],
		},
		{
			(analogSensor_t*) &thermistors [0][0],
			(analogSensor_t*) &thermistors [0][1],
			(analogSensor_t*) &thermistors [0][2],
			(analogSensor_t*) &thermistors [0][3],
			(analogSensor_t*) &thermistors [0][4],
		},
		{
			NULL, NULL, NULL, NULL, NULL
		},
		{
			NULL, NULL, NULL, NULL, NULL
		}
	},
};

static ltc6811_t* const DAISY_CHAIN [] =
{
	&ltcs [1],
	&ltcs [0],
	&ltcs [3],
	&ltcs [2],
	&ltcs [5],
	&ltcs [4],
	&ltcs [7],
	&ltcs [6],
	&ltcs [9],
	&ltcs [8],
	&ltcs [11],
	&ltcs [10]
};

static const thermistorPulldownConfig_t THERMISTOR_CONFIG =
{
	.steinhartHartA			= 1.1384e-3,
	.steinhartHartB			= 2.3245e-4,
	.steinhartHartC			= 0,
	.steinhartHartD			= 9.489e-8,
	.resistanceReference	= 1,
	.resistancePullup		= 10000,
	.temperatureMin			= -10,
	.temperatureMax			= 60
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
	ltc6811Init (DAISY_CHAIN, LTC_COUNT, &DAISY_CHAIN_CONFIG);
	ltcBottom = DAISY_CHAIN [0];

	return true;
}

void peripheralsReconfigure (void)
{
	// Thermistor initialization
	for (uint16_t deviceIndex = 0; deviceIndex < LTC_COUNT; ++deviceIndex)
		for (uint16_t gpioIndex = 0; gpioIndex < LTC6811_GPIO_COUNT; ++gpioIndex)
			thermistorPulldownInit (&thermistors [deviceIndex][gpioIndex], &THERMISTOR_CONFIG);
}

void peripheralsSample (void)
{
	// Sample the LTCs
	ltc6811ClearState (ltcBottom);
	ltc6811SampleCells (ltcBottom);
	ltc6811SampleCellVoltageSum (ltcBottom);
	ltc6811SampleCellVoltageFaults (ltcBottom);
	ltc6811SampleGpio (ltcBottom);
	ltc6811OpenWireTest (ltcBottom);

	// Update the global state

	packVoltage = 0.0f;
	for (uint16_t index = 0; index < LTC_COUNT; ++index)
		packVoltage += ltcs [index].cellVoltageSum;

	undervoltageFault = ltc6811UndervoltageFault (ltcBottom);
	overvoltageFault = ltc6811OvervoltageFault (ltcBottom);
	senseLineFault = ltc6811OpenWireFault (ltcBottom);
	isoSpiFault = ltc6811IsospiFault (ltcBottom);
	selfTestFault = ltc6811SelfTestFault (ltcBottom);

	undertemperatureFault = false;
	for (uint16_t ltcIndex = 0; ltcIndex < LTC_COUNT; ++ltcIndex)
		for (uint16_t thermistorIndex = 0; thermistorIndex < LTC6811_GPIO_COUNT; ++thermistorIndex)
			undertemperatureFault |= thermistors [ltcIndex][thermistorIndex].undertemperatureFault;

	overtemperatureFault = false;
	for (uint16_t ltcIndex = 0; ltcIndex < LTC_COUNT; ++ltcIndex)
		for (uint16_t thermistorIndex = 0; thermistorIndex < LTC6811_GPIO_COUNT; ++thermistorIndex)
			overtemperatureFault |= thermistors [ltcIndex][thermistorIndex].overtemperatureFault;
}