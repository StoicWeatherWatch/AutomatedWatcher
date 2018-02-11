/*
 * SW_Wind_Speed_Mean.cpp
 *
 *  Created on: 2018-02-06
 *      Author: StoicWeather
 */

#include "SW_Wind_Speed_Mean.h"

SW_Wind_Speed_Mean::SW_Wind_Speed_Mean(byte AddressIn, I2C I2CBussIn, byte NumberOfSpeedRecordsIn, byte SensorNumberIN)
:SW_MCP2318_GPIO_Sensor(AddressIn, I2CBussIn, SensorNumberIN)
{

	//  Wind Measurements
		HaveFullSpeedQueue = false;
		// Holds the location of the most recently recorded record in the queue
		CurrentSpeedQueueLoc = -1;
		// Counts by data points. With 13 bits it takes 2 bytes for each data point unless you want to get tricky.
		WindSpeedQueue = (byte*)calloc(NumberOfSpeedRecordsIn*2, sizeof(byte));
		SpeedQueueLength = NumberOfSpeedRecordsIn;


}

bool SW_Wind_Speed_Mean::AcquireData()
{
	// Prep queue
	CurrentSpeedQueueLoc++;

	// TODO CurrentQueueLoc >QueueLength >=? I think correct below
	if(CurrentSpeedQueueLoc >= SpeedQueueLength)
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
	if(CurrentSpeedQueueLoc != SpeedQueueLength - 1)
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
