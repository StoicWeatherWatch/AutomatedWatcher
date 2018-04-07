/*
 * SW_Wind_Dir_Mean.cpp
 *
 *  Created on: 2018-04-07
 *      Author: StoicWeather
 */

#include "SW_Wind_Dir_Mean.h"

SW_Wind_Dir_Mean::SW_Wind_Dir_Mean(byte AAQ0PinIn, byte NumberOfRecordsIn, byte SensorNumberAnalogIn)
:SW_Wind_Dir_Analog(AAQ0PinIn, NumberOfRecordsIn, SensorNumberAnalogIn)
{


}

void SW_Wind_Dir_Mean::SendMeanAndBinBlock()
{
#ifdef PRINT_TEST_LINES
	Serial.println(F("#SW_Wind_Dir_Mean SendMeanAndBinBlock;"));
#endif /*PRINT_TEST_LINES*/
	if(HaveFullDirectionQueue)
	{
#ifdef PRINT_TEST_LINES
	Serial.println(F("#SW_Wind_Dir_Mean SendMeanAndBinBlock HaveFullDirectionQueue;"));
#endif /*PRINT_TEST_LINES*/

#ifdef RUN_TIME_TEST
	unsigned long timeB = millis();
#endif

		// Bin the data
		//byte *Bins;
		//Bins = (byte*) calloc ((int)NUMBER_OF_BINS,sizeof(byte));

		//int *DirectionQueueCopy;
		//DirectionQueueCopy = (int*)calloc(DirectionQueueLength, sizeof(int));

		// Zero Bins
#ifdef PRINT_TEST_LINES
	Serial.println(F("#SW_Wind_Dir_Mean SendMeanAndBinBlock Zero Bins;"));
#endif /*PRINT_TEST_LINES*/
		for (int i = 0; i < NUMBER_OF_BINS; ++i)
		{
			Bins[i] = 0;
		}
#ifdef PRINT_TEST_LINES
	Serial.println(F("#SW_Wind_Dir_Mean SendMeanAndBinBlock Zero Bins Done;"));
#endif /*PRINT_TEST_LINES*/


		int DirectionReadingHolder;
		for(int i = 0; i < DirectionQueueLength; i++)
		{
			// We will need a temporary copy of the queue to rotate the compass
			DirectionReadingHolder = (int)(GetDirectionReadingAt(i));

			// Divide by 64 to place the data in 16 bins
			switch ((DirectionReadingHolder >> BITS_TO_SHIFT_FOR_BINNING))
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

		//byte *BinList;
		//BinList = (byte*) calloc ((int)NUMBER_OF_BINS,sizeof(byte));
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

	// TEST Lines
	/*Serial.println(F("# BinList ;"));
	Serial.print(F("# "));
	for(int i =0;i < NUMBER_OF_BINS; i++)
	{
		Serial.print(BinList[i],HEX);
		Serial.print(F(" "));
	}
	Serial.println(F(";"));
	Serial.println(F("# Bins ;"));
		Serial.print(F("# "));
		for(int i =0;i < NUMBER_OF_BINS; i++)
		{
			Serial.print(Bins[i],HEX);
			Serial.print(F(" "));
		}
		Serial.println(F(";"));*/



		// Rotate the compass
		// If we are in the first half, we add enough to get the largest bin to 512
		// In the upper half, add enough to get the largest bin to 1536
		int rotation;
		if(BinList[NUMBER_OF_BINS-1] >= (NUMBER_OF_BINS / 2))
		{
			rotation = 1535 - ((BinList[NUMBER_OF_BINS-1] << BITS_TO_SHIFT_FOR_BINNING) + HALF_BIN_SIZE);
		}
		else
		{
			rotation = 511 - ((BinList[NUMBER_OF_BINS-1] << BITS_TO_SHIFT_FOR_BINNING) + HALF_BIN_SIZE);
		}
	// Sum the data


		//TEST lines
		/*Serial.print(F("# bin rotation "));
			Serial.print(rotation);
			Serial.println(F(";"));*/

			long sum = 0;
			for(int i = 0; i < DirectionQueueLength; i++)
			{
				DirectionReadingHolder = (int)(GetDirectionReadingAt(i));

				DirectionReadingHolder += rotation;
				if(DirectionReadingHolder > MAX_ADC_VALUE)
				{
					DirectionReadingHolder -= (MAX_ADC_VALUE+1);
				}

				sum += DirectionReadingHolder;

			}





		// Send sum, rotation, bin info
			Serial.print(F("+"));
				Serial.print(SensorNumberAnalog,DEC);
				Serial.print(F("WMD,"));
				SerialHexFourAndAHalfBytefPrint(sum);
				Serial.print(F(","));
				SerialHexBytePrint(BinList[NUMBER_OF_BINS-1]);
				Serial.println(F(";"));

				// TODO Print Bin Info
				// Full width at half max. Sum up bins until you reach half the number of measurments. Report all bins in bin list.
				// If it is a normal dist, this will give FWHM. Not quite. Width at half the area under the curve.


#ifdef RUN_RAM_TEST
	extern int __heap_start, *__brkval;
		int v;
		Serial.print(F("# Free RAM in Wind Dir Mean "));
		Serial.print((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
		Serial.println(F(";"));
#endif

		//free(Bins);
		//free(BinList);
		//free(DirectionQueueCopy);

#ifdef RUN_TIME_TEST
	Serial.flush();
	unsigned long EndtimeB = millis();
	Serial.print(F("# Time to to take full mean "));
	Serial.print(EndtimeB-timeB);
	Serial.println(F(";"));
#endif
	} // End if have full dir queue
}

void SW_Wind_Dir_Mean::swap(byte *A, byte *B)
{
	byte temp = *A;
	*A = *B;
	*B = temp;
}
