/*
 * SW_Ard_Analog.h
 *
 *  Created on: 2018-01-31
 *      Author: StoicWeather
 *
 *      Super class for reading out a single analog pin
 *      */

#ifndef SW_ARD_ANALOG_H_
#define SW_ARD_ANALOG_H_

#include "Arduino.h"

#include "SW_Helper_Functions.h"

/*
 *
 */
class SW_Ard_Analog
{
public:
	byte SensorNumber;
	byte AAQ0Pin;
	SW_Ard_Analog(byte AAQ0PinIn, byte SensorNumberIn);
	virtual bool AcquireDataAndSend();
	int Read_Pin();
};

#endif /* SW_ARD_ANALOG_H_ */
