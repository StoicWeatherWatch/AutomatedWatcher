/*
 * SW_Wind_Dir_Analog.cpp
 *
 *  Created on: 2018-01-31
 *      Author: StoicWeather
 */

#include "SW_Wind_Dir_Analog.h"

/*
 * Storage format of DirectionQueue is LSB A [7:0] | MSB A [7:4] MSB B [3:0] | LSB B [7:0]
 * Even pointer numbers take the next half byte and tack it on high. Odd pointers take last half byte and tack it on high.
 * AAABBBCCCDDDEEEFFFGGG
 * 0011223344556677889900
 */

SW_Wind_Dir_Analog::SW_Wind_Dir_Analog(byte AAQ0PinIn, byte NumberOfRecordsIn, byte SensorNumberIn)
:SW_Ard_Analog(AAQ0PinIn, SensorNumberIn)
{
	HaveFullQueue = false;
	CurrentQueueLoc = -1;
	// Counts by data points. CurrentQueueLoc*3/2 + CurrentQueueLoc*3%2 gives pointer to LSB block.
	// We can fit a 10 bit ADC output in a 12 bit space. Thus we can fit two data points in three bytes. It just gets tricky to save and recover it...
	DirectionQueue = (byte*)calloc((NumberOfRecordsIn*3)/2, sizeof(byte));
	QueueLength = NumberOfRecordsIn;

	Serial.print(F("#Wind Direction Initialized;"));

}


bool SW_Wind_Dir_Analog::AcquireDataAndSend()
{
	int AAQread = analogRead(AAQ0Pin);

	Serial.print(F("*"));
	Serial.print(SensorNumber,DEC);
	Serial.print(F("WDINSTANT,"));
	SerialHexByteAndAHalfPrint(AAQread);
	Serial.println(F(";"));

	return RecordReading(AAQread);

}

bool SW_Wind_Dir_Analog::AcquireDataOnly()
{
	int AAQread = analogRead(AAQ0Pin);
	//Serial.println(F("#Wind AcquireDataOnly;"));

	return RecordReading(AAQread);

}

int SW_Wind_Dir_Analog::GetMostRecent()
{
	if(CurrentQueueLoc == -1)
	{
		// No data yet
		return 0;
	}

	byte Pointer = ((CurrentQueueLoc*3)/2) + ((CurrentQueueLoc*3)%2);

	int MostRecent = (int)DirectionQueue[Pointer];
	if(((CurrentQueueLoc*3)%2) == 0)
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
bool SW_Wind_Dir_Analog::RecordReading(int AAQread)
{

	//Serial.println(F("#Wind RecordReading;"));
	CurrentQueueLoc++;

	// TODO CurrentQueueLoc >QueueLength >= ?
	if(CurrentQueueLoc >= QueueLength)
	{
		CurrentQueueLoc = 0;
		HaveFullQueue = true;
	}
	/*Serial.print(F("#CurrentQueueLoc "));
	Serial.print(CurrentQueueLoc);
	Serial.println(F(";"));*/

	byte Pointer = ((CurrentQueueLoc*3)/2) + ((CurrentQueueLoc*3)%2);

	DirectionQueue[Pointer] = (byte)(AAQread & 255);

	if(((CurrentQueueLoc*3)%2) == 0)
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




	return true;

}

bool SW_Wind_Dir_Analog::SendAverage()
{
	if(!HaveFullQueue)
	{
		return false;
	}

	// TODO impliment

	return true;
}
bool SW_Wind_Dir_Analog::SendQueue()
{
	if(!HaveFullQueue)
	{
		Serial.println(F("#Wind SendQueue Queue not full;"));
		return false;
	}

#ifdef RUN_TIME_TEST
	unsigned long time = millis();
#endif

	Serial.print(F("*"));
	Serial.print(SensorNumber,DEC);
	Serial.print(F("WDQ,")); // Wind Direction Queue

	int QueueLocation = 0;
	byte Pointer = 0;
	int Reading = 0;

	for(int i = 0; i < QueueLength; i++)
	{
		int QueueLocation = CurrentQueueLoc + i;
		if(QueueLocation >= QueueLength)
		{
			QueueLocation -= QueueLength;

		}
	Pointer = ((QueueLocation*3)/2) + ((QueueLocation*3)%2);

		 Reading = (int)DirectionQueue[Pointer];
			if(((CurrentQueueLoc*3)%2) == 0)
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

			SerialHexByteAndAHalfPrint(Reading);

	}
	Serial.println(F(";"));

#ifdef RUN_TIME_TEST
	Serial.flush();
	unsigned long Endtime = millis();
	Serial.print(F("# Time to print "));
	Serial.print(Endtime-time);
	Serial.println(F(";"));
#endif



	return true;
}


