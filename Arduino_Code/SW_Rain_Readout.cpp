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

	FirstReadout = true;
	LastDataReading = (byte)0;



}

bool SW_Rain_Readout::AcquireDataAndSend()
{

	byte DataIn = SW_Ard_Readout::Read_Pins();

	if(!FirstReadout)
	{
		Serial.print("*");
		Serial.print(SensorNumber,DEC);
		Serial.print("R,");
		SerialHexBytePrint(DataIn);
		//Serial.print(DataIn,HEX);
		Serial.print(",");
		SerialHexBytePrint(LastDataReading);
		//Serial.print(LastDataReading,HEX);
		Serial.println(";");

		// TODO Add additional previous readings to add robustness against a missed line in serial

		// TEST lines
		//Serial.println(DataIn,HEX);
		//Serial.println(LastDataReading,HEX);

	}

	//Serial.println(DataIn,HEX);
	//Serial.println(LastDataReading,HEX);
	LastDataReading = DataIn;
	//Serial.println(DataIn,HEX);
	//Serial.println(LastDataReading,HEX);
	FirstReadout = false;

	return true;


}
