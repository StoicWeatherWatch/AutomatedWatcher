/*
 * SW_Wind_Dir_Mean.cpp
 *
 *  Created on: 2018-02-04
 *      Author: StoicWeather
 */

#include "SW_Wind_Dir_Mean.h"

SW_Wind_Dir_Mean::SW_Wind_Dir_Mean(byte AAQ0PinIn, byte NumberOfRecordsIn, byte SensorNumberAnalogIn)
:SW_Wind_Dir_Analog(AAQ0PinIn, NumberOfRecordsIn, SensorNumberAnalogIn)
{


}

void SW_Wind_Dir_Mean::SendMeanAndBinBlock()
{
	if(HaveFullDirectionQueue)
	{
		// Bin the data
		byte *Bins;
		Bins = (byte*) calloc ((int)NUMBER_OF_BINS,sizeof(byte));

		int *DirectionQueueCopy;
		DirectionQueueCopy = (int*)calloc(DirectionQueueLength, sizeof(int));

		for(int i = 0; i < DirectionQueueLength; i++)
		{
			// We will need a temporary copy of the queue to rotate the compass
			DirectionQueueCopy[i] = (int)(GetDirectionReadingAt(i));

			// Divide by 64 to place the data in 16 bins
			switch ((DirectionQueueCopy[i] >> BITS_TO_SHIFT_FOR_BINNING))
			{

			case 0 :
				Bins[0]++;
				break;
			case 1 :
				Bins[1]++;
				break;
			case 2 :
				Bins[2]++;
				break;
			case 3 :
				Bins[3]++;
				break;
			case 4:
				Bins[4]++;
				break;
			case 5 :
				Bins[5]++;
				break;
			case 6 :
				Bins[6]++;
				break;
			case 7 :
				Bins[7]++;
				break;
			case 8 :
				Bins[8]++;
				break;
			case 9 :
				Bins[9]++;
				break;
			case 10 :
				Bins[10]++;
				break;
			case 11 :
				Bins[11]++;
				break;
			case 12 :
				Bins[12]++;
				break;
			case 13 :
				Bins[13]++;
				break;
			case 14 :
				Bins[14]++;
				break;
			case 15 :
				Bins[15]++;
				break;
			}

		} // end for


		// Find most common bin

#ifdef RUN_TIME_TEST
	unsigned long time = millis();
#endif

		byte *BinList;
		BinList = (byte*) calloc ((int)NUMBER_OF_BINS,sizeof(byte));
		for(int i = 0; i < (int)NUMBER_OF_BINS; i++)
		{
			BinList[i] = (byte)i;
		}

		// Bubble sort :)
		int i, j;
		for (i = 0; i < ((int)NUMBER_OF_BINS)-1; i++)
		{
			for (j = 0; j < ((int)NUMBER_OF_BINS)-i-1; j++)
			{
				if (Bins[j] > Bins[j+1])
				{
					swap(&Bins[j], &Bins[j+1]);
					swap(&BinList[j], &BinList[j+1]);
				}
			}
		}

#ifdef RUN_TIME_TEST
	Serial.flush();
	unsigned long Endtime = millis();
	Serial.print(F("# Time to sort "));
	Serial.print(Endtime-time);
	Serial.println(F(";"));
#endif





		// Rotate the compass
	// Sum the data
	int rotation = (BinList[NUMBER_OF_BINS-1] << BITS_TO_SHIFT_FOR_BINNING) + HALF_BIN_SIZE;

		//TEST lines
		Serial.print(F("# bin rotation "));
			Serial.print(rotation);
			Serial.println(F(";"));

			long sum = 0;
			for(int i = 0; i < DirectionQueueLength; i++)
			{
				DirectionQueueCopy[i] += rotation;
				if(DirectionQueueCopy[i] > MAX_ADC_VALUE)
				{
					DirectionQueueCopy[i] -= MAX_ADC_VALUE+1;
				}

				sum += DirectionQueueCopy[i];

			}





		// Send sum, rotation, bin info
			Serial.print(F("*"));
				Serial.print(SensorNumberAnalog,DEC);
				Serial.print(F("WDM,"));
				SerialHexFourAndAHalfBytefPrint(sum);
				Serial.print(F(","));

						Serial.print(BinList[NUMBER_OF_BINS-1]);
				Serial.print(F(";"));

				// TODO Print Bin Info


#ifdef RUN_RAM_TEST
	extern int __heap_start, *__brkval;
		int v;
		Serial.print(F("# Free RAM in Wind Dir Mean "));
		Serial.print((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
		Serial.println(F(";"));
#endif

		free(Bins);
		free(BinList);
		free(DirectionQueueCopy);
	}
}

void SW_Wind_Dir_Mean::swap(byte *A, byte *B)
{
	byte temp = *A;
	*A = *B;
	*B = temp;
}
