/*
 * SW_Wind_Dir_Analog.h
 *
 *  Created on: 2018-01-31
 *      Author: StoicWeather
 */

#ifndef SW_WIND_DIR_ANALOG_H_
#define SW_WIND_DIR_ANALOG_H_

#include "SW_Ard_Analog.h"

#define RUN_TIME_TEST

/*
 * Storage format of DirectionQueue is LSB A [7:0] | MSB A [7:4] MSB B [3:0] | LSB B [7:0]
 * Even pointer numbers take the next half byte and tack it on high. Odd pointers take last half byte and tack it on high.
 */
class SW_Wind_Dir_Analog: public SW_Ard_Analog
{
public:
	byte *DirectionQueue;
	int CurrentQueueLoc;
	byte QueueLength; // Bytes
	bool HaveFullQueue;

	SW_Wind_Dir_Analog(byte AAQ0PinIn, byte NumberOfRecordsIn, byte SensorNumberIn);
	bool AcquireDataAndSend();
	bool AcquireDataOnly();
	int GetMostRecent();
	int GetReadingAt(byte TargetQueueLoc);
	bool RecordReading(int AAQread);
	bool SendAverage();
	bool SendQueue();
};

#endif /* SW_WIND_DIR_ANALOG_H_ */
