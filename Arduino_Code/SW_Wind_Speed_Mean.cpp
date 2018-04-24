/*
 * SW_Wind_Speed_Mean.cpp
 *
 *  Created on: 2018-04-07
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
	//WindSpeedQueue = (byte*)calloc(NumberOfSpeedRecordsIn*2, sizeof(byte));
	//SpeedQueueLength = NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP;

#ifdef RUN_TIME_TEST
	SW_Wind_Time_holder[0] = 0;
	SW_Wind_Time_holder[1] = 0;
	SW_Wind_Time_holder[2] = 0;
	SW_Wind_Time_holder[3] = 0;
	SW_Wind_Time_holder[4] = 0;
	SW_Wind_Time_holder[5] = 0;
	SW_Wind_Time_holder[6] = 0;
	SW_Wind_Time_holder[7] = 0;
	SW_Wind_Time_holder[8] = 0;
	SW_Wind_Time_holder[9] = 0;
	SW_Wind_Time_holder[10] = 0;
	SW_Wind_Time_holder[11] = 0;
	SW_Wind_Time_holder[12] = 0;
	SW_Wind_Time_holder[13] = 0;
	SW_Wind_Time_holder[14] = 0;
	SW_Wind_Time_holder[15] = 0;
	SW_Wind_Time_holder[16] = 0;
	SW_Wind_Time_holder[17] = 0;
	SW_Wind_Time_holder[18] = 0;
	SW_Wind_Time_holder[19] = 0;
	SW_Wind_Time_holder[20] = 0;
	SW_Wind_Time_holder[21] = 0;
	SW_Wind_Time_holder[22] = 0;
	SW_Wind_Time_holder[23] = 0;
#endif /*RUN_TIME_TEST*/
#ifdef RUN_LONG_TIME_TEST
	SW_Wind_Time_CURRENT = 0;
	SW_Wind_Time_LAST = 0;
	SW_Wind_Time_COUNT = 0;
#endif /*RUN_LONG_TIME_TEST*/
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
	if(CurrentSpeedQueueLoc >= (int)NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP)
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
	Serial.println(F("# SW_Wind_Speed_Mean AcquireDataAndReturn calling;"));
	int DataIn = AcquireDataAndReturn();

	WindSpeedQueue[CurrentSpeedQueueLoc] = DataIn;


#ifdef RUN_TIME_TEST
	SW_Wind_Time_holder[CurrentSpeedQueueLoc] = millis();
#endif /*RUN_TIME_TEST*/
#ifdef RUN_LONG_TIME_TEST

	SW_Wind_Time_CURRENT = millis();
	SW_Wind_Time_COUNT++;

	if(SW_Wind_Time_LAST == 0)
	{
		SW_Wind_Time_COUNT = 0;
		SW_Wind_Time_LAST = SW_Wind_Time_CURRENT;
	}

	if((SW_Wind_Time_CURRENT - SW_Wind_Time_LAST) >= 600000)
	{
		Serial.print(F("!SPEEDTIME, "));
		Serial.print(SW_Wind_Time_CURRENT - SW_Wind_Time_LAST);
		Serial.print(F(", "));
				Serial.print(SW_Wind_Time_COUNT);
		Serial.println(F(";"));
		SW_Wind_Time_LAST = SW_Wind_Time_CURRENT;
		SW_Wind_Time_COUNT = 0;
	}




#endif /*RUN_LONG_TIME_TEST*/

	return true;

}

// Returns counts. Divide by time periods to get speed
int SW_Wind_Speed_Mean::GetMostRecentRawMean()
{
	if(!HaveFullSpeedQueue)
	{
		//This should never happen since it should be tested before being called
		return 0;
	}

	byte OldestQueueLoc = 0;
	if(CurrentSpeedQueueLoc != (NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP - 1))
	{
		// This is the normal case.
		OldestQueueLoc = CurrentSpeedQueueLoc + 1;
	}
	else
	{
		OldestQueueLoc = 0;
	}

	int Difference = WindSpeedQueue[CurrentSpeedQueueLoc] - WindSpeedQueue[OldestQueueLoc];

	if(Difference < 0)
	{

#ifdef REPORT_SPEED_COUNTER_ROLLOVER
		Serial.println(F("#SW_Wind_Speed GetMostRecentRawMean Counter roll over;"));
		Serial.print(F("!SPEEDOVER "));
		Serial.print(WindSpeedQueue[CurrentSpeedQueueLoc]);
		Serial.print(F("  "));
		Serial.print(WindSpeedQueue[OldestQueueLoc]);
		Serial.print(F("  "));
				Serial.print(Difference);
		Serial.println(F(";"));
#endif /*REPORT_SPEED_COUNTER_ROLLOVER*/

		Difference = Difference + (int)MAX_WIND_CTS;
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

#ifdef REPORT_HIGH_MEAN
//7 m/s = 15 mph = 835 clicks in 120 seconds
	if(Difference >= (835))
	{
	Serial.println(F("#SW_Wind_Speed GetMostRecentRawMean High Speed Reported;"));
	Serial.print(F("!SPEEDHIGH "));
	Serial.print(WindSpeedQueue[CurrentSpeedQueueLoc]);
			Serial.print(F(",  "));
			Serial.print(WindSpeedQueue[OldestQueueLoc]);
	Serial.print(F(",  "));
	Serial.print(Difference);
	Serial.print(F(",  "));
	Serial.print(CurrentSpeedQueueLoc);
	Serial.print(F(",  "));
	Serial.print(OldestQueueLoc);
	Serial.println(F(";"));
	}
#endif /*REPORT_HIGH_MEAN*/

#ifdef RUN_TIME_TEST
	Serial.print(F("#SW_Wind_Speed GetMostRecentRawMean time between records "));
	Serial.print(SW_Wind_Time_holder[CurrentSpeedQueueLoc]);
	Serial.print(F(",  "));
		Serial.print(SW_Wind_Time_holder[OldestQueueLoc]);
		Serial.print(F(",  "));
		Serial.print(SW_Wind_Time_holder[CurrentSpeedQueueLoc]-SW_Wind_Time_holder[OldestQueueLoc]);
	Serial.println(F(";"));
#endif /*RUN_TIME_TEST*/
#ifdef RUN_TIME_TEST
	Serial.print(F("#SW_Wind_Speed GetMostRecentRawMean time between records "));
	Serial.print(SW_Wind_Time_holder[CurrentSpeedQueueLoc]);
	Serial.print(F(",  "));
		Serial.print(SW_Wind_Time_holder[OldestQueueLoc]);
		Serial.print(F(",  "));
		Serial.print(SW_Wind_Time_holder[CurrentSpeedQueueLoc]-SW_Wind_Time_holder[OldestQueueLoc]);
	Serial.println(F(";"));
#endif /*RUN_TIME_TEST*/


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
