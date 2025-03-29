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

typedef enum
{
	LTC6811_STATE_PEC_ERROR	= 0,
	LTC6811_STATE_READY		= 1
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

#endif // LTC6811_H