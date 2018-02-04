/*
 * SW_Ard_Analog.cpp
 *
 *  Created on: 2018-01-31
 *      Author: StoicWeather
 */

#include "SW_Ard_Analog.h"


SW_Ard_Analog::SW_Ard_Analog(byte AAQ0PinIn, byte SensorNumberIn)
:SensorNumber(SensorNumberIn),
 AAQ0Pin(AAQ0Pin)
{


}

int SW_Ard_Analog::Read_Pin()
{
	int ADCresult = analogRead((unsigned char)AAQ0Pin);

	return ADCresult;

}
