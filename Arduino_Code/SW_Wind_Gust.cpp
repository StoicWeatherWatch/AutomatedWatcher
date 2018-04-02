/*
 * SW_Wind_Gust.cpp
 *
 *  Created on: 2018-02-10
 *      Author: StoicWeather
 */

#include "SW_Wind_Gust.h"

SW_Wind_Gust::SW_Wind_Gust(byte MCP23018AddressIn, I2C I2CBussIn, byte NumberofGustSpeedRecordsIn, byte SpeedSensorNumberIN, byte AAQ0PinIn, byte NumberOfDirectionRecordsIn, byte DirectionSensorNumberIn)
:SW_MCP2318_GPIO_Sensor(MCP23018AddressIn, I2CBussIn, SpeedSensorNumberIN),
 SW_Wind_Dir_Analog(AAQ0PinIn, NumberOfDirectionRecordsIn, DirectionSensorNumberIn)
{
	int NumBytesInSentRecordQueue = ((int)NumberofGustSpeedRecordsIn) / 8;
	if((((int)NumberofGustSpeedRecordsIn) % 8) != 0)
		NumBytesInSentRecordQueue++;

	// TODO Replace all calloc with fixed array sizes fed from conditional includes.
	SentRecordQueue = (byte*)calloc(NumBytesInSentRecordQueue, sizeof(byte));

	//  Wind Measurements
	HaveFullSpeedQueue = false;
	// Holds the location of the most recently recorded record in the queue
	CurrentSpeedQueueLoc = -1;
	// Counts by data points. With 13 bits it takes 2 bytes for each data point unless you want to get tricky.
	// But since we only record differences, a byte is all we need
	// TODO Replace all calloc with fixed array sizes fed from conditional includes.
	WindSpeedGustQueue = (byte*)calloc(NumberofGustSpeedRecordsIn, sizeof(byte));
	/*
	 * For reasons unknown, calloc is not initializing the array to zero. And attempts to correct it caused all sorts of memory overflow issues.*/

	SpeedQueueLength = NumberofGustSpeedRecordsIn;

	LastGustReadout = 0;


}


void SW_Wind_Gust::AcquireWindGustDirection()
{
	AcquireDirectionDataOnly();

}

void SW_Wind_Gust::AcquireWindGustSpeed()
{
	// Prep queue
	CurrentSpeedQueueLoc++;

	// TODO CurrentQueueLoc >QueueLength >=?
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

	byte Difference;

	if(LastGustReadout > DataIn)
	{
		Difference = (byte)(DataIn + (int)MAX_WIND_CTS - LastGustReadout);
	}
	else
	{
		Difference = (byte)(DataIn - LastGustReadout);
	}

#ifdef TEST_PRINTS

	Serial.print(F("#Wind read  "));
	Serial.print(DataIn,HEX);
	Serial.print(F("  diff from last  "));
	Serial.print(Difference,HEX);
	Serial.println(F(";"));

#endif /*TEST_PRINTS*/

	LastGustReadout = DataIn;

#ifdef RUN_GUST_SIMULATION
	if((CurrentSpeedQueueLoc % 2) == 0)
	{
	Difference = 0;
	}
	else
	{
		Difference = (byte)50;
	}
#endif /*RUN_GUST_SIMULATION*/


	WindSpeedGustQueue[CurrentSpeedQueueLoc] = Difference;



	SetSentRecord(CurrentSpeedQueueLoc, false);


}

void SW_Wind_Gust::SendWindGustData()
{

#ifdef TEST_PRINTS
	// TODO why is most recent in first spot and second in last?
	Serial.println(F("#Wind Speed Queue;"));
	Serial.print(F("#"));
	byte QueueLoc = 0;
	for(int i = 0; i< SpeedQueueLength; i++)
	{
		QueueLoc = CurrentSpeedQueueLoc + i;

		// TODO CurrentQueueLoc >QueueLength >=?
		if(QueueLoc >= SpeedQueueLength)
		{
			QueueLoc -= SpeedQueueLength;
		}

		Serial.print(F(","));
		SerialHexBytePrint(WindSpeedGustQueue[QueueLoc]);

	}
	Serial.println(F(";"));

	// TEST BLOCK
	Serial.println(F("#SentRecordQueue;"));
	Serial.print(F("#"));

	for(int i = 0; i< SpeedQueueLength; i++)
	{
		QueueLoc = CurrentSpeedQueueLoc + i;

		// TODO CurrentQueueLoc >QueueLength >=?
		if(QueueLoc >= SpeedQueueLength)
		{
			QueueLoc -= SpeedQueueLength;
		}

		Serial.print(F(",  "));
		if(GustNotSentPreviously(QueueLoc))
		{
			Serial.print(F("0"));
		}
		else
		{
			Serial.print(F("1"));
		}


	}
	Serial.println(F(";"));


#endif /*TEST_PRINTS*/
if(HaveFullSpeedQueue)
{
	// Find min and max in queue
	int MaxPos = -1;
	//int MinPos = -1;
	byte minSpeed = 150;
	byte maxSpeed = 0;

	for(int i = 0; i < (int)SpeedQueueLength; i++)
	{
		if(WindSpeedGustQueue[i] < minSpeed)
		{
			minSpeed = WindSpeedGustQueue[i];
			//MinPos = i;
		}
		if(WindSpeedGustQueue[i] > maxSpeed)
		{
			maxSpeed = WindSpeedGustQueue[i];
			MaxPos = i;
		}
	}

	// TEST Block
#ifdef TEST_PRINTS

	Serial.print(F("# CUrrent , max, min"));
	Serial.print(WindSpeedGustQueue[CurrentSpeedQueueLoc]);
	Serial.print(F(","));
	Serial.print(maxSpeed);
	Serial.print(F(","));
	Serial.print(minSpeed);
	Serial.println(F(";"));
#endif /*TEST_PRINTS*/

	// Max must be 10 knots above min and not have been sent
	if(maxSpeed >= (minSpeed + (byte)GUST_MINIMUM_DIFFERENCE))
	{
		// TEST line
#ifdef TEST_PRINTS

		Serial.println(F(" # Max is above threshold;"));
#endif /*TEST_PRINTS*/

		if(GustNotSentPreviously(MaxPos))
		{
			// TEST line
#ifdef TEST_PRINTS

					Serial.println(F(" # Max has not been sent;"));
					#endif /*TEST_PRINTS*/
			// set sent to true
			SetSentRecord(MaxPos, true);
			// send data
			Serial.print(F("*"));
			Serial.print(SensorNumber,DEC);
			Serial.print(F("WGS,"));
			SerialHexBytePrint(WindSpeedGustQueue[MaxPos]);
			Serial.print(F(";"));

			Serial.print(F("+"));
			Serial.print(SensorNumberAnalog,DEC);
			Serial.print(F("WGD,"));
			SerialHexByteAndAHalfPrint(GetDirectionReadingAt((byte)MaxPos));
			Serial.println(F(";"));
		}
	}
}


}



// Returns true if this record has not been sent and false if it has.
bool SW_Wind_Gust::GustNotSentPreviously(byte QueueLocGust)
{
	byte ByteLoc = QueueLocGust / 8;
	byte BitLoc = QueueLocGust % 8;

	// make location mask
	byte LocationMask = 1 << BitLoc;

	byte BitValue = (SentRecordQueue[ByteLoc] & LocationMask);

	if(BitValue == 0)
		return true;
	else
		return false;


}

void SW_Wind_Gust::SetSentRecord(byte GustSpeedQueueLocToSet, bool HasBeenSent)
{


	byte ByteLoc = GustSpeedQueueLocToSet / 8;
	byte BitLoc = GustSpeedQueueLocToSet % 8;

	// make location mask
	byte LocationMask = 1 << BitLoc;

	// zero out the location
	SentRecordQueue[ByteLoc] &= ~LocationMask;

	// add 1 to location if true
	if(HasBeenSent)
	{
		SentRecordQueue[ByteLoc] += LocationMask;
	}

}
