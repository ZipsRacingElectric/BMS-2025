// Header
#include "peripherals.h"

// Includes
#include "peripherals/stm_adc.h"

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

// Public
mc24lc32_t				hardwareEeprom;
eepromMap_t*			hardwareEepromMap;
virtualEeprom_t			virtualEeprom;
ltc6811_t				ltcs [LTC_COUNT];
ltc6811_t*				ltcBottom;
thermistorPulldown_t	thermistors [LTC_COUNT][LTC6811_GPIO_COUNT];
dhabS124_t				currentSensor;

// Private
stmAdc_t				adc;
eeprom_t				readonlyWriteonlyEeprom;

// Configuration --------------------------------------------------------------------------------------------------------------

/// @brief Configuration for the I2C 1 bus.
static const I2CConfig I2C1_CONFIG =
{
	.op_mode		= OPMODE_I2C,
	.clock_speed	= 400000,
	.duty_cycle		= FAST_DUTY_CYCLE_2
};

/// @brief Configuration for the ADC 1 peripheral.
static const stmAdcConfig_t ADC_CONFIG =
{
	.driver			= &ADCD1,
	.channels		=
	{
		ADC_CHANNEL_IN0,
		ADC_CHANNEL_IN1
	},
	.channelCount	= 2,
	.sensors		=
	{
		(analogSensor_t*) &currentSensor.channel1,
		(analogSensor_t*) &currentSensor.channel2
	}
};

/// @brief Configuration for the hardware EEPROM.
static const mc24lc32Config_t HARDWARE_EEPROM_CONFIG =
{
	.addr			= 0x50,
	.i2c			= &I2CD1,
	.timeout		= TIME_MS2I (500),
	.magicString	= EEPROM_MAP_STRING
};

/// @brief Configuration for the BMS's virtual memory.
static const virtualEepromConfig_t VIRTUAL_EEPROM_CONFIG =
{
	.count		= 2,
	.entries	=
	{
		{
			.eeprom	= (eeprom_t*) &hardwareEeprom,
			.addr	= 0x0000,
			.size	= 0x1000,
		},
		{
			.eeprom = &readonlyWriteonlyEeprom,
			.addr	= 0x1000,
			.size	= 0x1000
		}
	}
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
	.readAttemptCount		= 5,							// Fail after 5 invalid read attempts.
	.cellAdcMode			= LTC6811_ADC_422HZ,			// TODO(Barach): Figure out 26 Hz. 26 Hz ADC sampling for cell voltages.
	.gpioAdcMode			= LTC6811_ADC_422HZ,			// TODO(Barach): Figure out 26 Hz. 26 Hz ADC sampling for the thermistors.
	.openWireTestIterations	= 3,							// Perform 3 pull-up / pull-down commands before measuring.
	.faultCount				= 6,							// Maximum of 6 continuous faults allowed. At a sampling rate of
															// 2 Hz, this is 3 seconds.
	.cellVoltageMax			= 4.16,							// Maximum voltage for the COSMX 95B0D0HD, any high exceeds a pack
															// voltage of 600V and is therefore illegal.
	.cellVoltageMin			= 3,							// Minimum voltage for the COSMX 95B0D0HD, any lower is below the
															// acceptable voltage range.
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
			(analogSensor_t*) &thermistors [3][0],
			(analogSensor_t*) &thermistors [3][1],
			(analogSensor_t*) &thermistors [3][2],
			(analogSensor_t*) &thermistors [3][3],
			(analogSensor_t*) &thermistors [3][4],
		},
		{
			(analogSensor_t*) &thermistors [2][0],
			(analogSensor_t*) &thermistors [2][1],
			(analogSensor_t*) &thermistors [2][2],
			(analogSensor_t*) &thermistors [2][3],
			(analogSensor_t*) &thermistors [2][4],
		},
		{
			(analogSensor_t*) &thermistors [5][0],
			(analogSensor_t*) &thermistors [5][1],
			(analogSensor_t*) &thermistors [5][2],
			(analogSensor_t*) &thermistors [5][3],
			(analogSensor_t*) &thermistors [5][4],
		},
		{
			(analogSensor_t*) &thermistors [4][0],
			(analogSensor_t*) &thermistors [4][1],
			(analogSensor_t*) &thermistors [4][2],
			(analogSensor_t*) &thermistors [4][3],
			(analogSensor_t*) &thermistors [4][4],
		},
		{
			(analogSensor_t*) &thermistors [7][0],
			(analogSensor_t*) &thermistors [7][1],
			(analogSensor_t*) &thermistors [7][2],
			(analogSensor_t*) &thermistors [7][3],
			(analogSensor_t*) &thermistors [7][4],
		},
		{
			(analogSensor_t*) &thermistors [6][0],
			(analogSensor_t*) &thermistors [6][1],
			(analogSensor_t*) &thermistors [6][2],
			(analogSensor_t*) &thermistors [6][3],
			(analogSensor_t*) &thermistors [6][4],
		},
		{
			(analogSensor_t*) &thermistors [9][0],
			(analogSensor_t*) &thermistors [9][1],
			(analogSensor_t*) &thermistors [9][2],
			(analogSensor_t*) &thermistors [9][3],
			(analogSensor_t*) &thermistors [9][4],
		},
		{
			(analogSensor_t*) &thermistors [8][0],
			(analogSensor_t*) &thermistors [8][1],
			(analogSensor_t*) &thermistors [8][2],
			(analogSensor_t*) &thermistors [8][3],
			(analogSensor_t*) &thermistors [8][4],
		},
		{
			(analogSensor_t*) &thermistors [11][0],
			(analogSensor_t*) &thermistors [11][1],
			(analogSensor_t*) &thermistors [11][2],
			(analogSensor_t*) &thermistors [11][3],
			(analogSensor_t*) &thermistors [11][4],
		},
		{
			(analogSensor_t*) &thermistors [10][0],
			(analogSensor_t*) &thermistors [10][1],
			(analogSensor_t*) &thermistors [10][2],
			(analogSensor_t*) &thermistors [10][3],
			(analogSensor_t*) &thermistors [10][4],
		}
	},
};

/// @brief The LTC IsoSPI daisy-chain, used to accomodate for changes to the IsoSPI wiring.
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

// Functions ------------------------------------------------------------------------------------------------------------------

bool peripheralsInit (void)
{
	// ADC 1 initialization
	if (!stmAdcInit (&adc, &ADC_CONFIG))
		return false;

	// I2C 1 driver initialization
	if (i2cStart (&I2CD1, &I2C1_CONFIG) != MSG_OK)
		return false;

	// Hardware EEPROM initialization
	if (!mc24lc32Init (&hardwareEeprom, &HARDWARE_EEPROM_CONFIG) && hardwareEeprom.state == MC24LC32_STATE_FAILED)
		return false;
	hardwareEepromMap = (eepromMap_t*) hardwareEeprom.cache;

	// Readonly / Writeonly EEPROM initialization
	eepromInit (&readonlyWriteonlyEeprom, eepromWriteonlyWrite, eepromReadonlyRead);

	// Virtual EEPROM initialization
	virtualEepromInit (&virtualEeprom, &VIRTUAL_EEPROM_CONFIG);

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
			thermistorPulldownInit (&thermistors [deviceIndex][gpioIndex], &hardwareEepromMap->thermistorConfig);

	// Current sensor initialization
	dhabS124Init (&currentSensor, &hardwareEepromMap->currentSensorConfig);
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

	// TODO(Barach): Re-implement
	undertemperatureFault = false;
	// for (uint16_t ltcIndex = 0; ltcIndex < LTC_COUNT; ++ltcIndex)
	// 	for (uint16_t thermistorIndex = 0; thermistorIndex < LTC6811_GPIO_COUNT; ++thermistorIndex)
	// 		undertemperatureFault |= thermistors [ltcIndex][thermistorIndex].undertemperatureFault;

	overtemperatureFault = false;
	for (uint16_t ltcIndex = 0; ltcIndex < LTC_COUNT; ++ltcIndex)
		for (uint16_t thermistorIndex = 0; thermistorIndex < LTC6811_GPIO_COUNT; ++thermistorIndex)
			overtemperatureFault |= thermistors [ltcIndex][thermistorIndex].overtemperatureFault;

	bmsFault = undervoltageFault || overtemperatureFault || senseLineFault || isoSpiFault || senseLineFault ||
		undertemperatureFault || overtemperatureFault;

	// Sample the current sensor
	stmAdcSample (&adc);
}