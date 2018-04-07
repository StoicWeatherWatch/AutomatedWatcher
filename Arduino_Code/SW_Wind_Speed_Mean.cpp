/*
 * SW_Wind_Speed_Mean.cpp
 *
 *  Created on: 2018-02-06
 *      Author: StoicWeather
 */

#include "SW_Wind_Speed_Mean.h"

SW_Wind_Speed_Mean::SW_Wind_Speed_Mean(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN)
:SW_MCP2318_GPIO_Sensor(AddressIn, I2CBussIn, SensorNumberIN)
{

	//  Wind Measurements
		HaveFullSpeedQueue = false;
		// Holds the location of the most recently recorded record in the queue
		CurrentSpeedQueueLoc = -1;
		// Counts by data points. With 13 bits it takes 2 bytes for each data point unless you want to get tricky.
		// TODO Replace all calloc with fixed array sizes fed from conditional includes.
		//WindSpeedQueue = (byte*)calloc(NumberOfSpeedRecordsIn*2, sizeof(byte));
		//SpeedQueueLength = NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP;


}

bool SW_Wind_Speed_Mean::AcquireData()
{
#ifdef TEST_REPORT_MEAN_STATUS
	Serial.println(F("# SW_Wind_Speed_Mean AcquireData;"));

	extern int __heap_start, *__brkval;
		int v;
		Serial.print(F("# Free RAM  "));
		Serial.print((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
		Serial.println(F(";"));
		// END TEST

#endif /* TEST_REPORT_MEAN_STATUS*/
	// Prep queue
	CurrentSpeedQueueLoc++;

	// TODO CurrentQueueLoc >QueueLength >=? I think correct below
	if(CurrentSpeedQueueLoc >= NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP)
	{
		CurrentSpeedQueueLoc = 0;
		HaveFullSpeedQueue = true;
	}

	/*//SEQOP = 0 So the second byte should be GPIOA :)
	byte status = I2CBuss.read(SensorAddress, (byte)MCP2318_GPIOB_REG, (byte)2);

	//TEST line
	Serial.print(F("# MCP2318 read status "));
	Serial.print(status);
	Serial.println(F(";"));

	int DataIn = (int)I2CBuss.receive();
	// Most sig bits came in first. They get shifted up. Only 13 bits of data.
	DataIn = DataIn &0b00011111;
	DataIn <<= 8;
	DataIn |= (int)I2CBuss.receive();

	//TEST LINE
	Serial.print(F("# MCP2318 readin "));
		Serial.print(DataIn, HEX);
		Serial.println(F(";"));
	 */
	int DataIn = AcquireDataAndReturn();

	WindSpeedQueue[CurrentSpeedQueueLoc*2] = (byte)(DataIn >> 8);

	// (byte) should truncate
	WindSpeedQueue[(CurrentSpeedQueueLoc*2)+1] = (byte)(DataIn);



	return true;

}

// Returns counts. Divide by time periods to get speed
int SW_Wind_Speed_Mean::GetMostRecentRawMean()
{
	if(!HaveFullSpeedQueue)
	{
		return 0;
	}

	byte OldestQueueLoc = 0;
	if(CurrentSpeedQueueLoc != NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP - 1)
	{
		// This is the normal case.
		OldestQueueLoc = CurrentSpeedQueueLoc + 1;
	}
	else
	{
		OldestQueueLoc = 0;
	}

	int Difference = (int)((WindSpeedQueue[CurrentSpeedQueueLoc*2] << 8) + WindSpeedQueue[(CurrentSpeedQueueLoc*2)+1])
					- (int)((WindSpeedQueue[OldestQueueLoc*2] << 8) + WindSpeedQueue[(OldestQueueLoc*2)+1]);

	if(Difference < 0)
	{
		Difference += MAX_WIND_CTS;
	}

#ifdef TEST_REPORT_MEAN_STATUS
	Serial.println(F("# SW_Wind_Speed_Mean GetMostRecentRawMean returning;"));

	extern int __heap_start, *__brkval;
		int v;
		Serial.print(F("# Free RAM  "));
		Serial.print((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
		Serial.println(F(";"));
		// END TEST

#endif /* TEST_REPORT_MEAN_STATUS*/

	return Difference;

}

void SW_Wind_Speed_Mean::SendMostRecentRawMean()
{
	if(HaveFullSpeedQueue)
	{


	int Difference = GetMostRecentRawMean();

	Serial.print(F("*"));
	Serial.print(SensorNumber,DEC);
	Serial.print(F("WMS,"));
	SerialHexByteAndAHalfPrint(Difference);
	Serial.print(F(";"));

	}

}
