#ifndef LTC6811_H
#define LTC6811_H

// LTC6811-1 Driver -----------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date Created: 2025.02.17
//
// Description: Device driver for the LTC6811-1 BMS ICs.
//
// Note: This code is derivative of the Analog Devices Linduino codebase:
//   https://github.com/analogdevicesinc/Linduino/tree/master.
//
// TODO(Barach): Combined GPIO and cell voltage command exists.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "peripherals/analog_sensor.h"

// ChibiOS
#include "hal.h"

// Constants ------------------------------------------------------------------------------------------------------------------

#define LTC6811_CELL_COUNT		12
#define LTC6811_GPIO_COUNT		5
#define LTC6811_BUFFER_SIZE		8

// Datatypes ------------------------------------------------------------------------------------------------------------------

// TODO(Barach): This doesn't support ADCOPT = 1 modes.
typedef enum
{
	LTC6811_ADC_422HZ	= 0b00,
	LTC6811_ADC_27KHZ	= 0b01,
	LTC6811_ADC_7KHZ	= 0b10,
	LTC6811_ADC_26HZ	= 0b11,
} ltc6811AdcMode_t;

/// @brief The maximum amount of time cells may be discharged without any update. TODO(Barach): There is no public way to
/// update or balance yet.
typedef enum
{
	LTC6811_DISCHARGE_TIMEOUT_DISABLED	= 0x0,
	LTC6811_DISCHARGE_TIMEOUT_30_S		= 0x1,
	LTC6811_DISCHARGE_TIMEOUT_1_MIN		= 0x2,
	LTC6811_DISCHARGE_TIMEOUT_2_MIN		= 0x3,
	LTC6811_DISCHARGE_TIMEOUT_3_MIN		= 0x4,
	LTC6811_DISCHARGE_TIMEOUT_4_MIN		= 0x5,
	LTC6811_DISCHARGE_TIMEOUT_5_MIN		= 0x6,
	LTC6811_DISCHARGE_TIMEOUT_10_MIN	= 0x7,
	LTC6811_DISCHARGE_TIMEOUT_15_MIN	= 0x8,
	LTC6811_DISCHARGE_TIMEOUT_20_MIN	= 0x9,
	LTC6811_DISCHARGE_TIMEOUT_30_MIN	= 0xA,
	LTC6811_DISCHARGE_TIMEOUT_40_MIN	= 0xB,
	LTC6811_DISCHARGE_TIMEOUT_60_MIN	= 0xC,
	LTC6811_DISCHARGE_TIMEOUT_75_MIN	= 0xD,
	LTC6811_DISCHARGE_TIMEOUT_90_MIN	= 0xE,
	LTC6811_DISCHARGE_TIMEOUT_120_MIN	= 0xF
} ltc6811DischargeTimeout_t;

typedef enum
{
	/// @brief Indicates a packet with an incorrect PEC was received. All other information about the device is void.
	LTC6811_STATE_PEC_ERROR = 0,

	/// @brief Indicates one or more cell voltages are not valid. Check @c overvoltageFaults , @c undervoltageFaults ,
	/// and @c openWireFaults for more details.
	LTC6811_STATE_CELL_FAULT = 1,

	/// @brief Indicates the device's multiplexor self test failed. All ADC measurements are void.
	LTC6811_STATE_SELF_TEST_FAULT = 2,

	/// @brief Indicates the device is operating normally.
	LTC6811_STATE_READY = 3
} ltc6811State_t;

typedef enum
{
	LTC6811_CHAIN_STATE_FAILED	= 0,
	LTC6811_CHAIN_STATE_READY	= 1,
} ltc6811ChainState_t;

typedef struct
{
	ltc6811State_t state;

	float cellVoltages [LTC6811_CELL_COUNT];
	float cellVoltagesPullup [LTC6811_CELL_COUNT];
	float cellVoltagesPulldown [LTC6811_CELL_COUNT];
	float cellVoltagesDelta [LTC6811_CELL_COUNT];

	bool overvoltageFaults [LTC6811_CELL_COUNT];
	bool undervoltageFaults [LTC6811_CELL_COUNT];
	bool openWireFaults [LTC6811_CELL_COUNT + 1];

	uint8_t tx [LTC6811_BUFFER_SIZE];
	uint8_t rx [LTC6811_BUFFER_SIZE];

	uint16_t vref2;
} ltc6811_t;

typedef struct
{
	/// @brief The SPI bus the daisy chain is connected to.
	SPIDriver* spiDriver;

	/// @brief The SPI configuration of the daisy chain.
	SPIConfig spiConfig;

	/// @brief The MISO line of the SPI bus.
	ioline_t spiMiso;

	/// @brief The array of @c ltc6811_t devices forming the daisy chain. No need to initialize the elements.
	ltc6811_t* devices;

	/// @brief The number of devices in the daisy chain, size of @c devices .
	uint16_t deviceCount;

	/// @brief The number of times to attempt a read operation before failing.
	uint16_t readAttemptCount;

	/// @brief The ADC conversion mode to use for measuring the cell voltages.
	ltc6811AdcMode_t cellAdcMode;

	/// @brief The ADC conversion mode to use for measuring the GPIO voltages.
	ltc6811AdcMode_t gpioAdcMode;

	/// @brief TODO(Barach)
	ltc6811DischargeTimeout_t dischargeTimeout;

	/// @brief The number of pull-up / pull-down command iterations to perform during the open wire test. This value should be
	/// determined through testing, but cannot be less than 2.
	uint8_t openWireTestIterations;

	/// @brief The minimum plausible cell voltage measurement, any lower indicates a fault condition.
	float cellVoltageMin;

	/// @brief The maximum plausible cell voltage measurement, any higher indicates a fault condition.
	float cellVoltageMax;

	/// @brief Indicates whether each GPIO ADC sensor is ratiometric with respect to VREF2. If true, samples will be scaled from
	// [0, 30000), where 30000 => VREF2.
	bool gpioAdcsRatiometric [LTC6811_GPIO_COUNT];

	/// @brief Multidimensional array of analog sensors to call upon sampling each device's GPIO. Must be size
	/// [ @c deviceCount ][ @c LTC6811_GPIO_COUNT ].
	analogSensor_t* gpioAdcSensors [][LTC6811_GPIO_COUNT];
} ltc6811DaisyChainConfig_t;

typedef struct
{
	ltc6811ChainState_t			state;
	ltc6811DaisyChainConfig_t*	config;
} ltc6811DaisyChain_t;

// Functions ------------------------------------------------------------------------------------------------------------------

bool ltc6811Init (ltc6811DaisyChain_t* chain, ltc6811DaisyChainConfig_t* config);

bool ltc6811SampleCells (ltc6811DaisyChain_t* chain);

bool ltc6811SampleGpio (ltc6811DaisyChain_t* chain);

bool ltc6811OpenWireTest (ltc6811DaisyChain_t* chain);

#endif // LTC6811_H