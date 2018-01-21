/*
 * SW_BME280_Sensor.h
 *
 *  Created on: 2018-01-21
 *      Author: StoicWeather
 */

#ifndef SW_BME280_SENSOR_H_
#define SW_BME280_SENSOR_H_

#include "SWSensor.h"

#define BME280_CHIPID_REG 0xD0
#define BME280_RESET_REG 0xE0

#define BME280_CHIPID_VALUE  0x60

#define BME280_RESETCOMMAND 0xB6

/*
 *
 */
class SW_BME280_Sensor: public SW_Sensor
{
public:
	SW_BME280_Sensor(byte AddressIn, I2C I2CBussIn);
	bool InitializeSensor();

#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/
};

#endif /* SW_BME280_SENSOR_H_ */
