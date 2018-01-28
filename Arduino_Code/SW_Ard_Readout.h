/*
 * SW_Ard_Readout.h
 *
 *  Created on: 2018-01-25
 *      Author: StoicWeather
 *
 *      Virtual superclass for DAQ from the Arduino digital and analog pins.
 *
 *      Limited in code to 8 data pins. Pins must be sequential. (Or override Read_Pins())
 */



#ifndef SW_ARD_READOUT_H_
#define SW_ARD_READOUT_H_

#include "Arduino.h"

#include "SW_Helper_Functions.h"

/*
 *
 */
class SW_Ard_Readout
{

public:
	byte SensorNumber;
	byte DAQ0Pin;
	byte DAQPinCount;
	byte ResetPin;




	SW_Ard_Readout(byte DAQ0PinIN, byte DAQPinCountIN,byte ResetPinIN,byte SensorNumberIN);
	bool setup();
	virtual bool AcquireDataAndSend();
	byte Read_Pins();
};

#endif /* SW_ARD_READOUT_H_ */
