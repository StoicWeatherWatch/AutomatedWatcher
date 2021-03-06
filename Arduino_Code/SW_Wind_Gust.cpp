/*
 * SW_Wind_Gust.cpp
 *
 *  Created on: 2018-04-07
 *      Author: StoicWeather
 */

#include "SW_Wind_Gust.h"

SW_Wind_Gust::SW_Wind_Gust(byte MCP23018AddressIn, I2C I2CBussIn, byte SpeedSensorNumberIN, byte AAQ0PinIn, byte DirectionSensorNumberIn)
:SW_MCP2318_GPIO_Sensor(MCP23018AddressIn, I2CBussIn, SpeedSensorNumberIN),
 SW_Wind_Dir_Analog(AAQ0PinIn, (byte)NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP, GustDirectionQueue, DirectionSensorNumberIn)
{
	//int NumBytesInSentRecordQueue = ((int)NumberofGustSpeedRecordsIn) / 8;
	/*if((((int)NumberofGustSpeedRecordsIn) % 8) != 0)
		NumBytesInSentRecordQueue++;*/


	//SentRecordQueue = (byte*)calloc(NumBytesInSentRecordQueue, sizeof(byte));

	//  Wind Measurements
	HaveFullSpeedQueue = false;
	// Holds the location of the most recently recorded record in the queue
	CurrentSpeedQueueLoc = -1;
	// Counts by data points. With 13 bits it takes 2 bytes for each data point unless you want to get tricky.
	// But since we only record differences, a byte is all we need

	//WindSpeedGustQueue = (byte*)calloc(NumberofGustSpeedRecordsIn, sizeof(byte));
	/*
	 * For reasons unknown, calloc is not initializing the array to zero. And attempts to correct it caused all sorts of memory overflow issues.*/

	//SpeedQueueLength = NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP;

	LastGustReadout = 0;

#ifdef RUN_GUST_TIME_TEST
	SW_GUST_Current_Time = 0;
	SW_GUST_Last_Time = 0;
#endif /*RUN_GUST_TIME_TEST*/

#ifdef RUN_GUST_LONG_TIME_TEST
	SW_Gust_Time_LAST = 0;
	SW_Gust_Time_CURRENT = 0;
	SW_Gust_Time_COUNT = 0;
#endif /*RUN_GUST_LONG_TIME_TEST*/


}


void SW_Wind_Gust::AcquireWindGustDirection()
{
#ifdef TEST_REPORT_STATUS
	Serial.println(F("#SW_Wind_Gust::AcquireWindGustDirection() starting;"));
#endif /*TEST_REPORT_STATUS*/

	AcquireDirectionDataOnly();

#ifdef TEST_REPORT_STATUS
	Serial.println(F("#SW_Wind_Gust::AcquireWindGustDirection() done;"));
#endif /*TEST_REPORT_STATUS*/

}

void SW_Wind_Gust::AcquireWindGustSpeed()
{
	// Prep queue
	CurrentSpeedQueueLoc++;

	// TODO CurrentQueueLoc >QueueLength >=?
#ifdef TEST_REPORT_STATUS
	Serial.print(F("#SW_Wind_Gust AcquireWindGustSpeed testing queue length;"));
	Serial.print(CurrentSpeedQueueLoc);

	Serial.println(F("  CurrentSpeedQueueLoc"));

extern int __heap_start, *__brkval;
	int v;
	Serial.print(F("# Free RAM  "));
	Serial.print((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
	Serial.println(F(";"));
	// END TEST

#endif /* TEST_REPORT_STATUS*/
	if(CurrentSpeedQueueLoc >= (int)NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP)
	{
		CurrentSpeedQueueLoc = 0;
		HaveFullSpeedQueue = true;
	}

#ifdef TEST_REPORT_STATUS
	Serial.println(F("#SW_Wind_Gust AcquireWindGustSpeed testing queue length done;"));
#endif /* TEST_REPORT_STATUS*/

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
	Serial.println(F("#SW_Wind_Gust AcquireDataAndReturn Calling;"));
	int DataIn = AcquireDataAndReturn();

	byte Difference;

	//Test to see if the counter has rolled over
	if(LastGustReadout > DataIn)
	{
		Difference = (byte)(DataIn + ((int)MAX_WIND_CTS) - LastGustReadout);

#ifdef REPORT_GUST_COUNTER_ROLLOVER
		Serial.println(F("#SW_Wind_Gust AcquireWindGustSpeed Counter roll over;"));
		Serial.print(F("!GUSTOVER "));
		Serial.print(DataIn);
		Serial.print(F("  "));
		Serial.print(Difference);
		Serial.println(F(";"));
#endif /*REPORT_GUST_COUNTER_ROLLOVER*/

	}
	else
	{
		Difference = (byte)(DataIn - LastGustReadout);
	}

#ifdef TEST_PRINT_EACH_READ

	Serial.print(F("#SW_Wind_Gust read  "));
	Serial.print(DataIn,HEX);
	Serial.print(F("  diff from last  "));
	Serial.print(Difference,HEX);
	Serial.println(F(";"));

#endif /*TEST_PRINT_EACH_READ*/

#ifdef REPORT_HIGH_GUST
//7 m/s = 15 mph = 15 clicks in 2.25 seconds
	if(Difference >= (15))
	{
	Serial.println(F("#SW_Wind_Gust AcquireWindGustSpeed High Speed Reported;"));
	Serial.print(F("!GUSTHIGH "));
	Serial.print((int)Difference);
	Serial.print(F(",  "));
	Serial.print(DataIn);
	Serial.print(F(",  "));
	Serial.print(LastGustReadout);
	Serial.println(F(";"));
	}
#endif /*REPORT_HIGH_GUST*/

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

#ifdef RUN_GUST_TIME_TEST
	SW_GUST_Current_Time = millis();
	Serial.print(F("#SW_Wind_Gust AcquireWindGustSpeed time between records "));
	Serial.print(SW_GUST_Current_Time);
	Serial.print(F(",  "));
		Serial.print(SW_GUST_Last_Time);
		Serial.print(F(",  "));
		Serial.print(SW_GUST_Current_Time-SW_GUST_Last_Time);
	Serial.println(F(";"));
	SW_GUST_Last_Time = SW_GUST_Current_Time;
#endif /*RUN_GUST_TIME_TEST*/

#ifdef RUN_GUST_LONG_TIME_TEST

	SW_Gust_Time_CURRENT = millis();
	SW_Gust_Time_COUNT++;

	if(SW_Gust_Time_LAST == 0)
	{
		SW_Gust_Time_COUNT = 0;
		SW_Gust_Time_LAST = SW_Gust_Time_CURRENT;
	}

	if((SW_Gust_Time_CURRENT - SW_Gust_Time_LAST) >= 600000)
	{
		Serial.print(F("!GUSTTIME, "));
		Serial.print(SW_Gust_Time_CURRENT - SW_Gust_Time_LAST);
		Serial.print(F(", "));
				Serial.print(SW_Gust_Time_COUNT);
		Serial.println(F(";"));
		SW_Gust_Time_LAST = SW_Gust_Time_CURRENT;
		SW_Gust_Time_COUNT = 0;
	}




#endif /*RUN_GUST_LONG_TIME_TEST*/


} /*AcquireWindGustSpeed*/

void SW_Wind_Gust::SendWindGustData()
{

#ifdef TEST_PRINTS
	// TODO why is most recent in first spot and second in last?
	Serial.println(F("#Wind Speed Queue;"));
	Serial.print(F("#"));
	byte QueueLoc = 0;
	for(int i = 0; i< NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP; i++)
	{
		QueueLoc = CurrentSpeedQueueLoc + i;

		// TODO CurrentQueueLoc >QueueLength >=?
		if(QueueLoc >= NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP)
		{
			QueueLoc -= NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP;
		}

		Serial.print(F(","));
		SerialHexBytePrint(WindSpeedGustQueue[QueueLoc]);

	}
	Serial.println(F(";"));

	// TEST BLOCK
	Serial.println(F("#SentRecordQueue;"));
	Serial.print(F("#"));

	for(int i = 0; i< NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP; i++)
	{
		QueueLoc = CurrentSpeedQueueLoc + i;

		// TODO CurrentQueueLoc >QueueLength >=?
		if(QueueLoc >= NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP)
		{
			QueueLoc -= NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP;
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

#ifdef TEST_REPORT_STATUS
	Serial.println(F("#SW_Wind_Gust AcquireWindGustSpeed if(HaveFullSpeedQueue);"));
#endif /* TEST_REPORT_STATUS*/
	if(HaveFullSpeedQueue)
{
#ifdef TEST_REPORT_STATUS
	Serial.println(F("#SW_Wind_Gust AcquireWindGustSpeed if(HaveFullSpeedQueue) true;"));
#endif /* TEST_REPORT_STATUS*/

	// Find min and max in queue
	int MaxPos = -1;
	//int MinPos = -1;
	byte minSpeed = 150;
	byte maxSpeed = 0;

	for(int i = 0; i < (int)NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP; i++)
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
#ifdef TEST_REPORT_STATUS
	Serial.println(F("#SW_Wind_Gust AcquireWindGustSpeed if(HaveFullSpeedQueue) done;"));
#endif /* TEST_REPORT_STATUS*/

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
