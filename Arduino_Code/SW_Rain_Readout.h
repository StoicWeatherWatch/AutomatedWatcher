/*
 * SW_Rain_Readout.h
 *
 *  Created on: 2018-01-27
 *      Author: StoicWeather
 */

#ifndef SW_RAIN_READOUT_H_
#define SW_RAIN_READOUT_H_

#include "SW_Ard_Readout.h"

/*
 *
 */

// TODO weeWX pauses its readout and then restarts the driver to make the reports.
// Maybe reset counter after each readout? We know the arduino will be reset when the serial connection is reset
// Then we could use the count to get rain accumulated

class SW_Rain_Readout: public SW_Ard_Readout
{

public:
	bool FirstReadout;

	byte LastDataReading;




	SW_Rain_Readout(byte DAQ0PinIN, byte DAQPinCountIN,byte ResetPinIN,byte SensorNumberIN);
	bool AcquireDataAndSend();

};

#endif /* SW_RAIN_READOUT_H_ */
