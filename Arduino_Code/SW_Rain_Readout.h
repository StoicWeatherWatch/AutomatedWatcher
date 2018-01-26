/*
 * SW_Rain_Readout.h
 *
 *  Created on: Jan 25, 2018
 *      Author: jwellhouse
 */

#ifndef SW_RAIN_READOUT_H_
#define SW_RAIN_READOUT_H_

#include "SW_Ard_Readout.h"

/*
 *
 */
class SW_Rain_Readout: public SW_Ard_Readout
{

public:
	bool FirstReadout;

	byte LastDataReading;




	SW_Rain_Readout(byte DAQ0PinIN, byte DAQPinCountIN,byte ResetPinIN,byte SensorNumberIN);
	bool AcquireDataAndSend();

};

#endif /* SW_RAIN_READOUT_H_ */
