/*
 * SW_MCP9808_Sensor.h
 *
 *  Created on: 2018-01-21
 *      Author: StoicWeather
 */

#ifndef SW_MCP9808_SENSOR_H_
#define SW_MCP9808_SENSOR_H_

#include "SWSensor.h"

#define MCP9808_CHIPID_REG    0x07
#define MCP9808_MANID_REG     0x06

#define MCP9808_CHIPID_VALUE  0x0400
#define MCP9808_MANID_VALUE   0x0054

/*
 *
 */
class SW_MCP9808_Sensor: public SW_Sensor
{
public:
	SW_MCP9808_Sensor(byte AddressIn, I2C I2CBussIn);
	bool InitializeSensor();
#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/
};

#endif /* SW_MCP9808_SENSOR_H_ */
