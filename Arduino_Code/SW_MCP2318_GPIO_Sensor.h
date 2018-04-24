/*
 * SW_MCP2318_GPIO_Sensor.h
 *
 *  Created on: 2018-02-01
 *      Author: StoicWeather
 */
/*
 * NWS Wind gust rapid fluctuations of wind speed with a variation of 10 knots or more between peaks and lulls.
 * The gust speed is the max speed recorded. 10 knots is 11.5 clicks in 2.25 seconds.
 *
 * Data conventions
 * W for wind speed
 * WA for all wind (The entire mean queue)
 * WM for a raw mean. Devide by sample period to get actual mean
 * WG for gust in raw Current, min, Max
 *
 */

#ifndef SW_MCP2318_GPIO_SENSOR_H_
#define SW_MCP2318_GPIO_SENSOR_H_

//#define REPORT_ALL_READINGS_TEST
//#define SET_FULL_TESTS
//#define REPORT_MEMORY_LEVEL
#define SET_REPORT_BIT_READ

// These assume BANK = 0
#define MCP2318_IOCON_REG    0x0A
#define MCP2318_IODIRA_REG   0x00
#define MCP2318_GPIOA_REG    0x12
#define MCP2318_GPIOB_REG    0x13


#define MCP2318_IOCON_DEFAULT_VALUE 	 	0x0000
#define MCP2318_IODIRA_DEFAULT_VALUE  	0xFFFF

//Bank  = 0 MIRROR = 0 SEQOP = 0
// SEQOP may cause issues
#define MCP2318_CONFIG_IOCON_CMD    0x00





#include "SWSensor.h"
#include "SW_Helper_Functions.h"



/*
 *
 */
class SW_MCP2318_GPIO_Sensor: public SW_Sensor
{
public:


	SW_MCP2318_GPIO_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);

	int AcquireDataAndReturn();

// TODO make this static?
	bool InitializeSensor();



#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/
};

#endif /* SW_MCP2318_GPIO_SENSOR_H_ */
