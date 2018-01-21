/*
 * SWSensor.h
 *
 *  Created on: 2018-01-18
 *      Author: StoicWeather
 */

#ifndef SWSENSOR_H_
#define SWSENSOR_H_

#include "Arduino.h"

#include "I2C.h"

//Set to run a verification that the chips on each I2C address are the ones expected.
// Runs the verify function in each sensor in initialize.
#define VERIFY_CHIPS

/*
 *
 */
class SW_Sensor
{
	protected:




	public:
	byte SensorAddress;
	I2C I2CBuss;

	SW_Sensor(byte AddressIn, I2C I2CBussIn);
	byte SW_SensorGetAddress();

#ifdef VERIFY_CHIPS
	virtual bool VerifyChip();
#endif /*VERIFY_CHIPS*/


};

#endif /* SWSENSOR_H_ */
