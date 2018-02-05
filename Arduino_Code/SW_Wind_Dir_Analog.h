/*
 * SW_Wind_Dir_Analog.h
 *
 *  Created on: 2018-01-31
 *      Author: StoicWeather
 */

#ifndef SW_WIND_DIR_ANALOG_H_
#define SW_WIND_DIR_ANALOG_H_

#include "SW_Ard_Analog.h"

// Tests to see how llong it takes to serial write certain things.
#define RUN_TIME_TEST


// Make certain it is called the correct number of times
#define CALL_COUNT_VALIDATION

#define NUMBER_OF_GUST_RECORDS  4


/*
 * Storage format of DirectionQueue is LSB A [7:0] | MSB A [7:4] MSB B [3:0] | LSB B [7:0]
 * Even pointer numbers take the next half byte and tack it on high. Odd pointers take last half byte and tack it on high.
 */
class SW_Wind_Dir_Analog: public SW_Ard_Analog
{
public:
	byte *DirectionQueue;
	int CurrentDirectionQueueLoc;
	byte DirectionQueueLength; // data points
	bool HaveFullDirectionQueue;

	SW_Wind_Dir_Analog(byte AAQ0PinIn, byte NumberOfRecordsIn, byte SensorNumberAnalogIn);
	bool AcquireAnalogDataAndSend();

	bool AcquireDirectionDataOnly(); // Used in Wind Gust
	int GetMostRecentDirection();
	int GetDirectionReadingAt(byte TargetQueueLoc); // Used in Wind Gust
	bool RecordDirectionReading(int AAQread);
	bool SendDirectionAverage();
	bool SendDirectionQueue();


};

#endif /* SW_WIND_DIR_ANALOG_H_ */
