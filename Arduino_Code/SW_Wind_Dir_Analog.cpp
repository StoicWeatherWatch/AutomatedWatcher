/*
 * SW_Wind_Dir_Analog.cpp
 *
 *  Created on: 2018-02-08
 *      Author: StoicWeather
 */

#include "SW_Wind_Dir_Analog.h"

/*
 * Storage format of DirectionQueue is LSB A [7:0] | MSB A [7:4] MSB B [3:0] | LSB B [7:0]
 * Even pointer numbers take the next half byte and tack it on high. Odd pointers take last half byte and tack it on high.
 * AAABBBCCCDDDEEEFFFGGG
 * 0011223344556677889900
 */

SW_Wind_Dir_Analog::SW_Wind_Dir_Analog(byte AAQ0PinIn, byte NumberOfRecordsIn, byte SensorNumberAnalogIn)
:SW_Ard_Analog(AAQ0PinIn, SensorNumberAnalogIn)
{
	HaveFullDirectionQueue = false;
	CurrentDirectionQueueLoc = -1;
	// Counts by data points. CurrentQueueLoc*3/2 + CurrentQueueLoc*3%2 gives pointer to LSB block.
	// We can fit a 10 bit ADC output in a 12 bit space. Thus we can fit two data points in three bytes. It just gets tricky to save and recover it...
	DirectionQueue = (byte*)calloc((NumberOfRecordsIn*3)/2, sizeof(byte));
	DirectionQueueLength = NumberOfRecordsIn;


	Serial.print(F("#Wind Direction Initialized;"));


}



bool SW_Wind_Dir_Analog::AcquireDirectionDataOnly()
{
	int AAQread = Read_Pin();
	//Serial.println(F("#Wind AcquireDataOnly;"));

	return RecordDirectionReading(AAQread);

}

int SW_Wind_Dir_Analog::GetMostRecentDirection()
{
	// TODO this may never be called
	Serial.println(F("#SW_Wind_Dir_Analog GetMostRecentDirection ???????????????????????????????;"));


	if(CurrentDirectionQueueLoc == -1)
	{
		// No data yet
		return 0;
	}

	byte Pointer = ((CurrentDirectionQueueLoc*3)/2) + ((CurrentDirectionQueueLoc*3)%2);

	int MostRecent = (int)DirectionQueue[Pointer];
	if(((CurrentDirectionQueueLoc*3)%2) == 0)
		{
			Pointer++;
			// Will want to change 1792 if using a ADC with more than 10 bits
			MostRecent += ((int)(DirectionQueue[Pointer] & 240)) << 4;

		}
		else
		{
			Pointer--;
			MostRecent += ((int)(DirectionQueue[Pointer] & 15)) << 8;
		}

	return MostRecent;


}
bool SW_Wind_Dir_Analog::RecordDirectionReading(int AAQread)
{


	CurrentDirectionQueueLoc++;

	// TODO CurrentQueueLoc >QueueLength >= ?
	if(CurrentDirectionQueueLoc >= DirectionQueueLength)
	{
		CurrentDirectionQueueLoc = 0;
		HaveFullDirectionQueue = true;
	}
	/*Serial.print(F("#CurrentQueueLoc "));
	Serial.print(CurrentQueueLoc);
	Serial.println(F(";"));*/

	byte Pointer = ((CurrentDirectionQueueLoc*3)/2) + ((CurrentDirectionQueueLoc*3)%2);

	DirectionQueue[Pointer] = (byte)(AAQread & 255);

	if(((CurrentDirectionQueueLoc*3)%2) == 0)
	{
		Pointer++;
		// Will want to change 1792 if using a ADC with more than 10 bits
		DirectionQueue[Pointer] = (DirectionQueue[Pointer] & 15) + ((byte)((AAQread & 1792) >> 4));

	}
	else
	{
		Pointer--;
		DirectionQueue[Pointer] = (DirectionQueue[Pointer] & 240) + ((byte)((AAQread & 1792) >> 8));
	}

// TEST LINES
	/*Serial.println(F("#Direction queue;"));
	Serial.print(F("# "));
	for(int i = 0;i<(DirectionQueueLength*3)/2; i++)
	{
		SerialHexBytePrint(DirectionQueue[i]);
		Serial.print(F(" "));
	}

		Serial.println(F(";"));*/

	return true;

}



int SW_Wind_Dir_Analog::GetDirectionReadingAt(byte TargetQueueLoc)
{

		byte Pointer = ((TargetQueueLoc*3)/2) + ((TargetQueueLoc*3)%2);


			int Reading = (int)DirectionQueue[Pointer];


			if(((TargetQueueLoc)%2) == 0)
				{

					Pointer++;


					// Will want to change 1792 if using a ADC with more than 10 bits
					Reading += ((int)(DirectionQueue[Pointer] & 240)) << 4;



				}
				else
				{
					Pointer--;



					Reading += ((int)(DirectionQueue[Pointer] & 15)) << 8;


				}

			return Reading;

}


