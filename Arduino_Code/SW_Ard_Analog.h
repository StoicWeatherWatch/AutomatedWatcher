/*
 * SW_Ard_Analog.h
 *
 *  Created on: 2018-02-10
 *      Author: StoicWeather
 *
 *      Super class for reading out a single analog pin
 *      */

#ifndef SW_ARD_ANALOG_H_
#define SW_ARD_ANALOG_H_

//#define RUN_SIMULATOR
//#define SIM_VALUE 0

#include "Arduino.h"

#include "SW_Helper_Functions.h"

/*
 *
 */
class SW_Ard_Analog
{
public:
	byte SensorNumberAnalog;
	byte AAQ0Pin;
	SW_Ard_Analog(byte AAQ0PinIn, byte SensorNumberIn);
	int Read_Pin();
};

#endif /* SW_ARD_ANALOG_H_ */
