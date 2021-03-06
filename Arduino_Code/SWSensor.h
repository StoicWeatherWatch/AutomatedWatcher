/*
 * SWSensor.h
 *
 *  Created on: 2018-01-18
 *      Author: StoicWeather
 *
 *      This is a virtual superclass for I2C sensors for the Stoic Watcher weather station.
 */

// TODO add an alert ! when a sensor is not reachable.

#ifndef SWSENSOR_H_
#define SWSENSOR_H_

#include "Arduino.h"

#include "I2C.h"

//Set to run a verification that the chips on each I2C address are the ones expected.
// Runs the verify function in each sensor in initialize.
#define VERIFY_CHIPS

//Set to perform conversion from raw bits to measurement values.
// Unset to leave that to higher level processor. This may be preferred as there is no double.
//#define PERFORM_DATA_PROCESSING

#ifdef PERFORM_DATA_PROCESSING
	#include "math.h"
#endif /*PERFORM_DATA_PROCESSING*/

// Set to send calibration data via serial at startup
//#define SEND_CALIBRATION_DATA

/*
 *
 */
class SW_Sensor
{
	protected:


	public:
	byte SensorAddress;
	I2C I2CBuss;
	//This number prefixes each data line sent by serial.
	byte SensorNumber;

	SW_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);





};

#endif /* SWSENSOR_H_ */
