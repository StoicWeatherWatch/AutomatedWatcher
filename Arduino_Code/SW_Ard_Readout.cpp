/*
 * SW_Ard_Readout.cpp
 *
 *  Created on: 2018-01-25
 *      Author: StoicWeather
 */

#include "SW_Ard_Readout.h"

SW_Ard_Readout::SW_Ard_Readout(byte DAQ0PinIN, byte DAQPinCountIN,byte ResetPinIN,byte SensorNumberIN)
 :SensorNumber(SensorNumberIN),
  DAQ0Pin(DAQ0PinIN),
  DAQPinCount(DAQPinCountIN),
  ResetPin(ResetPinIN)
{
	FirstReadout = true;
	LastDataReading = (byte)0;
}

// Set the pin modes and read the starting values from which we count
// On the first count, we do not report data since the timing is off.
bool SW_Ard_Readout::setup()
{
	FirstReadout = true;

	for(int i = 0; i < (int)DAQPinCount; i++)
	{
		pinMode((int)DAQ0Pin + i, INPUT);
	}

	return true;
}

byte SW_Ard_Readout::Read_Pins()
{
	/* There is a small chance that a bit will flip as we are reading. This will cause chaos.
	 * We accept a slightly slower readout to avoid such chaos. For example if we read low or high
	 * the next wind gust might be off. If we read high by more than 1, the next readout give a negative rain fall.
	*/
	byte CurrentRead = (byte)0;
	byte LastRead = (byte)0;

	// We want it run twice in all conditions. It is messy but I did not feel like chaning the counter inside a for loop
	for(int pin = DAQ0Pin; pin < DAQ0Pin + DAQPinCount; pin++ )
			{
				CurrentRead << 1;
				CurrentRead += (byte)digitalRead(pin);
			}

	do
	{
		LastRead = CurrentRead;
		CurrentRead = (byte)0;
		for(int pin = DAQ0Pin; pin < DAQ0Pin + DAQPinCount; pin++ )
		{
			CurrentRead << 1;
			CurrentRead += (byte)digitalRead(pin);
		}
	}
	while(CurrentRead != LastRead);

	return CurrentRead;

}
