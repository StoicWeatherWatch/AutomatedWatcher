/*
 * SW_Rain_Readout.cpp
 *
 *  Created on: Jan 25, 2018
 *      Author: jwellhouse
 */

#include "SW_Rain_Readout.h"

SW_Rain_Readout::SW_Rain_Readout(byte DAQ0PinIN, byte DAQPinCountIN,byte ResetPinIN,byte SensorNumberIN)
 :SW_Ard_Readout(DAQ0PinIN, DAQPinCountIN, ResetPinIN, SensorNumberIN)
{


}

bool SW_Rain_Readout::AcquireDataAndSend()
{

	byte DataIn = SW_Ard_Readout::Read_Pins();

	if(!FirstReadout)
	{
		Serial.print("*");
		Serial.print(SensorNumber,DEC);
		Serial.print("R,");
		Serial.print(DataIn,HEX);
		Serial.print(",");
		Serial.print(LastDataReading,HEX);
		Serial.println(";");
	}

	LastDataReading = DataIn;

	return true;


}
