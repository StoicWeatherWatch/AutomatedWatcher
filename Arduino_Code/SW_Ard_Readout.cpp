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


}

// Set the pin modes and read the starting values from which we count
// On the first count, we do not report data since the timing is off.
bool SW_Ard_Readout::setup()
{

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

	//Serial.print("#Read_Pins ");
	// We want it run twice in all conditions. It is messy but I did not feel like chaning the counter inside a for loop
	// Same code block below in While to repeat until they match
	for(int pin = DAQ0Pin + DAQPinCount -1; pin >= DAQ0Pin; pin--)
			{
				CurrentRead = CurrentRead << 1;
				CurrentRead = CurrentRead + (byte)digitalRead(pin);
				// TODO Replace the digital read with direct port access.



				//Test lines
				//Serial.print(" p ");
				//Serial.print(pin);
				//Serial.print(" st ");
				//Serial.print(digitalRead(pin));
				//Serial.print(" cr ");
				//Serial.print(CurrentRead);
			}
	//Serial.println(";");

	do
	{
		//Serial.print("#same_Pins ");
		LastRead = CurrentRead;
		CurrentRead = (byte)0;
		for(int pin = DAQ0Pin + DAQPinCount -1; pin >= DAQ0Pin; pin--)
		{
			CurrentRead = CurrentRead << 1;
			CurrentRead += (byte)digitalRead(pin);

			//Test lines
			/*Serial.print("p ");
			Serial.print(pin);
			Serial.print(" st ");
			Serial.print(digitalRead(pin));
			Serial.print(" cr ");
			Serial.print(CurrentRead);*/
		}
		//Serial.println(";");
	}
	while(CurrentRead != LastRead);

	//Test Lines
	//Serial.print("TestRead ");
	//byte aRead = (digitalRead(9)*1) + (digitalRead(10)*2) + (digitalRead(11)*4) + (digitalRead(12)*8);
	//Serial.println(aRead,HEX);

	return CurrentRead;

}
