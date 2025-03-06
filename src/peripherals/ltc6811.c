// Header
#include "ltc6811.h"

// Constants / Macros ---------------------------------------------------------------------------------------------------------

#define T_READY_MAX						TIME_US2I (10)
#define T_WAKE_MAX						TIME_US2I (400)

#define COMMAND_WRCFGA					0b00000000001
#define COMMAND_RDCFGA					0b00000000010

#define COMMAND_RDCVA					0b00000000100
#define COMMAND_RDCVB					0b00000000110
#define COMMAND_RDCVC					0b00000001000
#define COMMAND_RDCVD					0b00000001010

#define COMMAND_RDAUXA					0b00000001100
#define COMMAND_RDAUXB					0b00000001110

#define COMMAND_RDSTATA					0b00000010000
#define COMMAND_RDSTATB					0b00000010010

#define COMMAND_WRSCTRL					0b00000010100
#define COMMAND_RDSCTRL					0b00000010110

#define COMMAND_WRPWM					0b00000100000
#define COMMAND_RDPWM					0b00000100010

#define COMMAND_STSCTRL					0b00000011001
#define COMMAND_CLRSCTRL				0b00000011000

#define COMMAND_ADCV(ch, md, dcp)		(0b01001100000 | ((md) << 7) | ((dcp) << 4) | (ch))
#define COMMAND_ADOW(ch, md, dcp, pup)	(0b01000101000 | ((md) << 7) | ((dcp) << 4) | (ch) | ((pup) << 6))
#define COMMAND_CVST(md, st)			(0b01000000111 | ((md) << 7) | ((st) << 5))
#define COMMAND_ADOL(md, dcp)			(0b01000000001 | ((md) << 7) | ((dcp) << 4))

/// @brief Buffer to read/write irrelevant data from/to. Used in SPI transactions where the data doesn't matter.
static uint8_t nullBuffer [LTC6811_BUFFER_SIZE];

/// @brief Lookup table for calculating a frame's PEC. (See @c calculatePec )
static const uint16_t PEC_LUT [] =
{
	0x0000, 0xC599, 0xCEAB, 0x0B32, 0xD8CF, 0x1D56, 0x1664, 0xD3FD,
	0xF407, 0x319E, 0x3AAC, 0xFF35, 0x2CC8, 0xE951, 0xE263, 0x27FA,
	0xAD97, 0x680E, 0x633C, 0xA6A5, 0x7558, 0xB0C1, 0xBBF3, 0x7E6A,
	0x5990, 0x9C09, 0x973B, 0x52A2, 0x815F, 0x44C6, 0x4FF4, 0x8A6D,
	0x5B2E, 0x9EB7, 0x9585, 0x501C, 0x83E1, 0x4678, 0x4D4A, 0x88D3,
	0xAF29, 0x6AB0, 0x6182, 0xA41B, 0x77E6, 0xB27F, 0xB94D, 0x7CD4,
	0xF6B9, 0x3320, 0x3812, 0xFD8B, 0x2E76, 0xEBEF, 0xE0DD, 0x2544,
	0x02BE, 0xC727, 0xCC15, 0x098C, 0xDA71, 0x1FE8, 0x14DA, 0xD143,
	0xF3C5, 0x365C, 0x3D6E, 0xF8F7, 0x2B0A, 0xEE93, 0xE5A1, 0x2038,
	0x07C2, 0xC25B, 0xC969, 0x0CF0, 0xDF0D, 0x1A94, 0x11A6, 0xD43F,
	0x5E52, 0x9BCB, 0x90F9, 0x5560, 0x869D, 0x4304, 0x4836, 0x8DAF,
	0xAA55, 0x6FCC, 0x64FE, 0xA167, 0x729A, 0xB703, 0xBC31, 0x79A8,
	0xA8EB, 0x6D72, 0x6640, 0xA3D9, 0x7024, 0xB5BD, 0xBE8F, 0x7B16,
	0x5CEC, 0x9975, 0x9247, 0x57DE, 0x8423, 0x41BA, 0x4A88, 0x8F11,
	0x057C, 0xC0E5, 0xCBD7, 0x0E4E, 0xDDB3, 0x182A, 0x1318, 0xD681,
	0xF17B, 0x34E2, 0x3FD0, 0xFA49, 0x29B4, 0xEC2D, 0xE71F, 0x2286,
	0xA213, 0x678A, 0x6CB8, 0xA921, 0x7ADC, 0xBF45, 0xB477, 0x71EE,
	0x5614, 0x938D, 0x98BF, 0x5D26, 0x8EDB, 0x4B42, 0x4070, 0x85E9,
	0x0F84, 0xCA1D, 0xC12F, 0x04B6, 0xD74B, 0x12D2, 0x19E0, 0xDC79,
	0xFB83, 0x3E1A, 0x3528, 0xF0B1, 0x234C, 0xE6D5, 0xEDE7, 0x287E,
	0xF93D, 0x3CA4, 0x3796, 0xF20F, 0x21F2, 0xE46B, 0xEF59, 0x2AC0,
	0x0D3A, 0xC8A3, 0xC391, 0x0608, 0xD5F5, 0x106C, 0x1B5E, 0xDEC7,
	0x54AA, 0x9133, 0x9A01, 0x5F98, 0x8C65, 0x49FC, 0x42CE, 0x8757,
	0xA0AD, 0x6534, 0x6E06, 0xAB9F, 0x7862, 0xBDFB, 0xB6C9, 0x7350,
	0x51D6, 0x944F, 0x9F7D, 0x5AE4, 0x8919, 0x4C80, 0x47B2, 0x822B,
	0xA5D1, 0x6048, 0x6B7A, 0xAEE3, 0x7D1E, 0xB887, 0xB3B5, 0x762C,
	0xFC41, 0x39D8, 0x32EA, 0xF773, 0x248E, 0xE117, 0xEA25, 0x2FBC,
	0x0846, 0xCDDF, 0xC6ED, 0x0374, 0xD089, 0x1510, 0x1E22, 0xDBBB,
	0x0AF8, 0xCF61, 0xC453, 0x01CA, 0xD237, 0x17AE, 0x1C9C, 0xD905,
	0xFEFF, 0x3B66, 0x3054, 0xF5CD, 0x2630, 0xE3A9, 0xE89B, 0x2D02,
	0xA76F, 0x62F6, 0x69C4, 0xAC5D, 0x7FA0, 0xBA39, 0xB10B, 0x7492,
	0x5368, 0x96F1, 0x9DC3, 0x585A, 0x8BA7, 0x4E3E, 0x450C, 0x8095
};

// Function Prototypes --------------------------------------------------------------------------------------------------------

/**
 * @brief Calculates the packet error code given a frame's contents.
 * @param data The data to calculate the code for.
 * @param dataCount The number of elements in @c data .
 * @return The calculated PEC word. Note this value is already shifted to 0 the LSB.
 */
uint16_t calculatePec (uint8_t* data, uint8_t dataCount);

/**
 * @brief Checks whether the packet error code of a frame is correct.
 * @param data The data to validate the code of.
 * @param dataCount The number of elements in @c data .
 * @param pec The PEC to validate against.
 * @return True if the PEC is correct, false otherwise.
 */
bool validatePec (uint8_t* data, uint8_t dataCount, uint16_t pec);

/**
 * @brief Wakes up all devices in an LTC6811 daisy-chain. This method guarantees all devices are in the ready or standby state,
 * regardless of the previous state of the daisy-chain.
 * @param chain The daisy-chain to wake up.
 */
void wakeupDaisyChainSleep (ltc6811DaisyChain_t* chain);

/**
 * @brief Writes to a data register group of each device in a chain.
 * @note The to written to each device should be placed into its @c tx buffer.
 * @param chain The chain to write to.
 * @param command The write command of the register group.
 * @return True if successful, false otherwise.
 */
bool writeRegisterGroups (ltc6811DaisyChain_t* chain, uint16_t command);

/**
 * @brief Reads from a data register group of each device in a chain.
 * @note The data read from each device is placed into its @c rx buffer.
 * @param chain The chain to read from.
 * @param command The read command of the register group.
 * @return True if successful, false otherwise.
 */
bool readRegisterGroups (ltc6811DaisyChain_t* chain, uint16_t command);

// Functions ------------------------------------------------------------------------------------------------------------------

bool ltc6811Init (ltc6811DaisyChain_t* chain, ltc6811DaisyChainConfig_t* config)
{
	chain->config = config;

	for (uint16_t index = 0; index < chain->config->deviceCount; ++index)
		chain->config->devices [index].state = LTC6811_STATE_READY;

	// TODO(Barach): Check and initializations.
	return true;
}

void ltc6811ChainWriteTest (ltc6811DaisyChain_t* chain)
{
	// Acquire the SPI bus and start the driver.
	#if SPI_USE_MUTUAL_EXCLUSION
	spiAcquireBus (chain->config->spiDriver);
	#endif // SPI_USE_MUTUAL_EXCLUSION
	spiStart (chain->config->spiDriver, &chain->config->spiConfig);

	wakeupDaisyChainSleep (chain);

	// Write a pattern to a place where it won't cause any damage.
	for (uint16_t index = 0; index < chain->config->deviceCount; ++index)
	{
		chain->config->devices [index].tx [0] = 0x00;
		chain->config->devices [index].tx [1] = index;									// VUV
		chain->config->devices [index].tx [2] = chain->config->deviceCount - index - 1;	// VUV / VOV
		chain->config->devices [index].tx [3] = 0xAB;
		chain->config->devices [index].tx [4] = 0xCD;
		chain->config->devices [index].tx [5] = 0x00;
	}

	volatile bool result = writeRegisterGroups (chain, COMMAND_WRCFGA);
	result = result;
	__BKPT ();

	// Release the SPI bus
	spiStop (chain->config->spiDriver);
	#if SPI_USE_MUTUAL_EXCLUSION
	spiReleaseBus (chain->config->spiDriver);
	#endif // SPI_USE_MUTUAL_EXCLUSION
}

void ltc6811ChainReadTest (ltc6811DaisyChain_t* chain)
{
	// Acquire the SPI bus and start the driver.
	#if SPI_USE_MUTUAL_EXCLUSION
	spiAcquireBus (chain->config->spiDriver);
	#endif // SPI_USE_MUTUAL_EXCLUSION
	spiStart (chain->config->spiDriver, &chain->config->spiConfig);

	wakeupDaisyChainSleep (chain);

	// Read the data back and check what it reads.
	volatile bool result = readRegisterGroups (chain, COMMAND_RDCFGA);
	result = result;
	__BKPT ();

	// Release the SPI bus
	spiStop (chain->config->spiDriver);
	#if SPI_USE_MUTUAL_EXCLUSION
	spiReleaseBus (chain->config->spiDriver);
	#endif // SPI_USE_MUTUAL_EXCLUSION
}

uint16_t calculatePec (uint8_t* data, uint8_t dataCount)
{
	// The Packet Error Code (PEC) is a 15-bit CRC, generated using the following polynomial:
	//   x^15 + x^14 + x^10 + x^8 + x^7 + x^4 + x^3 + 1
	//
	// See LTC6811 datasheet, pg.53 for more info.

	// Begin with 0b0000 0000 0001 0000 (seed value)
	uint16_t remainder = 0x0010;

	// Traverse each byte of the payload, calculating the remainder using a lookup table.
	for (uint8_t index = 0; index < dataCount; ++index)
	{
		uint8_t addr = ((remainder >> 7) ^ data [index]);
		remainder = (remainder << 8) ^ PEC_LUT [addr];
	}

	// The LSB of the PEC word is a 0, so left shift once to align it.
	return (remainder << 1);
}

bool validatePec (uint8_t* data, uint8_t dataCount, uint16_t pec)
{
	// TODO(Barach): There is probably a more efficient way to validate this.
	uint16_t actualPec = calculatePec (data, dataCount);
	return pec == actualPec;
}

void wakeupDaisyChainSleep (ltc6811DaisyChain_t* chain)
{
	// Sends a wakeup signal using the algorithm described in "Waking a Daisy Chain - Method 2"
	// See LTC6811 datasheet, pg.52 for more info.

	// Wake each device individually. If a device in the stack is not in the ready state, it won't propogate the first signal
	// it receives, rather it will wake up and enter the ready state. Once said device is in the ready state, it will propogate
	// the next signal it receives. By sending N signals, we are allowing the first N - 1 devices to not propogate the first
	// signal they receive, in turn guaranteeing that all N devices will receive at least 1 signal.
	for (uint16_t index = 0; index < chain->config->deviceCount; ++index)
	{
		// Drive CS low for the maximum wakeup time (guarantees this device will wake).
		spiSelect (chain->config->spiDriver);
		chThdSleep (T_WAKE_MAX);

		// Release CS and allow the device to enter the ready state.
		spiUnselect (chain->config->spiDriver);
		chThdSleep (T_READY_MAX);
	}
}

bool writeRegisterGroups (ltc6811DaisyChain_t* chain, uint16_t command)
{
	// Transmit Frame:
	//  0            1            2        3
	// ---------------------------------------------
	// | Command HI | Command LO | PEC HI | PEC LO |
	// ---------------------------------------------
	//  4               5                     9               10       11
	// -------------------------------------------------------------------------
	// | Dev N-1 Reg 0 | Dev N-1 Reg 1 | ... | Dev N-1 Reg 5 | PEC HI | PEC LO |
	// -------------------------------------------------------------------------
	//  12              13                    17              18       19
	// -------------------------------------------------------------------------
	// | Dev N-2 Reg 0 | Dev N-2 Reg 1 | ... | Dev N-2 Reg 5 | PEC HI | PEC LO |
	// -------------------------------------------------------------------------
	// Where bytes [12, 19] are repeated down to device 0.

	// Write the command word followed by the PEC word.
	uint8_t tx [sizeof (uint16_t) * 2] = { command >> 8, command };
	uint16_t pec = calculatePec (tx, 2);
	tx [2] = pec >> 8;
	tx [3] = pec;

	spiSelect (chain->config->spiDriver);

	if (spiExchange (chain->config->spiDriver, sizeof (tx), tx, nullBuffer) != MSG_OK)
	{
		spiUnselect (chain->config->spiDriver);
		chain->state = LTC6811_CHAIN_STATE_FAILED;
		return false;
	}

	// Write each individual device's register group.
	// Note the first written data goes to the last device in the stack (device N-1).
	for (int16_t index = chain->config->deviceCount - 1; index >= 0; --index)
	{
		pec = calculatePec (chain->config->devices [index].tx, LTC6811_BUFFER_SIZE - sizeof(uint16_t));
		chain->config->devices [index].tx [LTC6811_BUFFER_SIZE - 2] = pec >> 8;
		chain->config->devices [index].tx [LTC6811_BUFFER_SIZE - 1] = pec;

		if (spiExchange (chain->config->spiDriver, LTC6811_BUFFER_SIZE, chain->config->devices [index].tx, nullBuffer)
			!= MSG_OK)
		{
			spiUnselect (chain->config->spiDriver);
			chain->state = LTC6811_CHAIN_STATE_FAILED;
			return false;
		}
	}

	spiUnselect (chain->config->spiDriver);

	return true;
}

bool readRegisterGroups (ltc6811DaisyChain_t* chain, uint16_t command)
{
	// See LTC6811 datasheet, pg.58 for more info.

	// Transmit Frame:
	//  0            1            2        3
	// ---------------------------------------------
	// | Command HI | Command LO | PEC HI | PEC LO |
	// ---------------------------------------------
	//
	// Receive Frame:
	//  0             1                   5             6        7
	// -------------------------------------------------------------------
	// | Dev 0 Reg 0 | Dev 0 Reg 1 | ... | Dev 0 Reg 5 | PEC HI | PEC LO |
	// -------------------------------------------------------------------
	//  8             9                   13            14       15
	// -------------------------------------------------------------------
	// | Dev 1 Reg 0 | Dev 1 Reg 1 | ... | Dev 1 Reg 5 | PEC HI | PEC LO |
	// -------------------------------------------------------------------
	// Where bytes [8, 15] are repeated up to device N-1.

	for (uint16_t attempt = 0; attempt < chain->config->readAttemptCount; ++attempt)
	{
		// TODO(Barach): The control of this is messy.

		// Write the command word followed by the PEC word.
		uint8_t tx [sizeof (uint16_t) * 2] = { command >> 8, command };
		uint16_t pec = calculatePec (tx, 2);
		tx [2] = pec >> 8;
		tx [3] = pec;

		spiSelect (chain->config->spiDriver);

		if (spiExchange (chain->config->spiDriver, sizeof (tx), tx, nullBuffer) != MSG_OK)
		{
			// If a SPI error occurs, something has failed inside the STM, re-attempting will not help.
			spiUnselect (chain->config->spiDriver);
			chain->state = LTC6811_CHAIN_STATE_FAILED;
			return false;
		}

		// Read each individual device's register group.
		// Note the first read data comes from the first device in the stack (device 0).
		for (uint16_t index = 0; index < chain->config->deviceCount; ++index)
		{
			if (spiExchange (chain->config->spiDriver, LTC6811_BUFFER_SIZE, nullBuffer, chain->config->devices [index].rx)
				!= MSG_OK)
			{
				// If a SPI error occurs, something has failed inside the STM, re-attempting will not help.
				spiUnselect (chain->config->spiDriver);
				chain->state = LTC6811_CHAIN_STATE_FAILED;
				return false;
			}
		}

		spiUnselect (chain->config->spiDriver);

		// Validate the PEC of each device's frame.
		bool result = true;
		for (uint16_t index = 0; index < chain->config->deviceCount; ++index)
		{
			pec = chain->config->devices [index].rx [LTC6811_BUFFER_SIZE - 1] |
				(chain->config->devices [index].rx [LTC6811_BUFFER_SIZE - 2] << 8);

			if (!validatePec (chain->config->devices [index].rx, LTC6811_BUFFER_SIZE - 2, pec))
			{
				result = false;

				// If this is not the last attempt, re-attempt the entire operation.
				if (attempt != chain->config->readAttemptCount - 1)
					break;

				// If this is the last attempt, fail the device and continue checking the others.
				chain->config->devices [index].state = LTC6811_STATE_PEC_ERROR;
				chain->state = LTC6811_CHAIN_STATE_FAILED;
			}
		}

		// If each device's PEC was valid, success.
		if (result)
			return true;
	}

	// Last attempt failed, fail the whole operation.
	return false;
}