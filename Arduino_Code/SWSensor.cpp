/*
 * SWSensor.cpp
 *
 *  Created on: 2018-01-18
 *      Author: StoicWeather
 */

#include "SWSensor.h"

SW_Sensor::SW_Sensor(byte AddressIn,I2C I2CBussIn)
:SensorAddress(AddressIn),
 I2CBuss(I2CBussIn)
{


}

byte SW_Sensor::SW_SensorGetAddress()
{
	return SW_Sensor::SensorAddress;
}
