/*
 * SW_Wind_Dir_Analog.h
 *
 *  Created on: 2018-04-07
 *      Author: StoicWeather
 */

#ifndef SW_WIND_DIR_ANALOG_H_
#define SW_WIND_DIR_ANALOG_H_

#include "SW_Ard_Analog.h"

// Tests to see how llong it takes to serial write certain things.
//#define RUN_TIME_TEST

// Print out each direction reading
//#define PRINT_READING_TEST


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
	//TODO make this void. Make DirectionQueue in SW_Wind_Dir_Mean and SW_Wind_Gust. That way it is not dynamic and length can be set with constant
	//TODO remove calloc see above
	/* DirectionQueue is a pointer is for an array of bytes length (3/2) * number of records. This provides 1.5 bytes, 12 bits, of data for each record.
	 * To avoid using calloc, which caused no end of chaos, this is fed a pointer to an array with a hard coded length by the sub-class.
	*/
	byte *DirectionQueue;
	int CurrentDirectionQueueLoc;
	byte DirectionQueueLength; // data points
	bool HaveFullDirectionQueue;

	SW_Wind_Dir_Analog(byte AAQ0PinIn, byte NumberOfRecordsIn, byte *PointerToDirectionQueueArrayIn, byte SensorNumberAnalogIn);
	bool AcquireDirectionDataOnly(); // Used in Wind Gust
	int GetMostRecentDirection();  // Is this ever called?
	int GetDirectionReadingAt(byte TargetQueueLoc); // Used in Wind Gust
	bool RecordDirectionReading(int AAQread); // Used in Wind mean




};

#endif /* SW_WIND_DIR_ANALOG_H_ */
