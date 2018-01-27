/*
 * SW_MCP9808_Sensor.h
 *
 *  Created on: 2018-01-21
 *      Author: StoicWeather
 *
 *      For maximum temperature resolution this sensor takes 250 ms to reach conversion. Res +0.0625°C
 */

#ifndef SW_MCP9808_SENSOR_H_
#define SW_MCP9808_SENSOR_H_

#include "SWSensor.h"
#include "SW_Helper_Functions.h"

#define MCP9808_CHIPID_REG    0x07
#define MCP9808_MANID_REG     0x06
#define MCP9808_RES_REG       0x08
#define MCP9808_CONFIG_REG    0x01
#define MCP9808_TEMP_REG      0x05

#define MCP9808_CHIPID_VALUE  0x0400
#define MCP9808_MANID_VALUE   0x0054

#define MCP9808_MAXRES_CMD    0x03
//0000 0000 0000 0000
#define MCP9808_CONFIG_MSB_CMD    0x00
#define MCP9808_CONFIG_LSB_CMD    0x00



/*
 *
 */
class SW_MCP9808_Sensor: public SW_Sensor
{
	protected:
	byte TemperatureRaw[2];

	public:


	SW_MCP9808_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);
	bool InitializeSensor();
	bool AcquireData();
	bool SendRawDataSerial();

	byte GetRawTempreature_HighBits();
	byte GetRawTempreature_LowBits();

#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/
#ifdef PERFORM_DATA_PROCESSING
	float ProcessTemp();
#endif /*PERFORM_DATA_PROCESSING*/
};

#endif /* SW_MCP9808_SENSOR_H_ */
