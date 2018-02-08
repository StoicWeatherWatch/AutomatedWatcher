/*
 * SW_Ard_Analog.cpp
 *
 *  Created on: 2018-02-08
 *      Author: StoicWeather
 */

#include "SW_Ard_Analog.h"


SW_Ard_Analog::SW_Ard_Analog(byte AAQ0PinIn, byte SensorNumberIn)
:SensorNumberAnalog(SensorNumberIn),
 AAQ0Pin(AAQ0PinIn)
{


}

int SW_Ard_Analog::Read_Pin()
{
	int ADCresult = analogRead((unsigned char)AAQ0Pin);

#ifdef RUN_SIMULATOR

	ADCresult = 592;
	Serial.println(F("# RUN_SIMULATOR ;"));

#endif /*RUN_SIMULATOR*/

	return ADCresult;

}
