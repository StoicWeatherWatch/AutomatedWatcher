/*
 * SW_ChipCap2_Sensor.h
 *
 *  Created on: 2018-03-16
 *      Author: StoicWeather
 */

#ifndef SW_CHIPCAP2_SENSOR_H_
#define SW_CHIPCAP2_SENSOR_H_

#include "SWSensor.h"
#include "SW_Helper_Functions.h"

/*
 *
 */
class SW_ChipCap2_Sensor: public SW_Sensor
{
public:
	SW_ChipCap2_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);
};

#endif /* SW_CHIPCAP2_SENSOR_H_ */
