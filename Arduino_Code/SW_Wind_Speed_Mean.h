/*
 * SW_Wind_Speed_Mean.h
 *
 *  Created on: 2018-04-12
 *      Author: StoicWeather
 */

#ifndef SW_WIND_SPEED_MEAN_H_
#define SW_WIND_SPEED_MEAN_H_

#include "SW_MCP2318_GPIO_Sensor.h"



//#define TEST_REPORT_MEAN_STATUS

#define REPORT_SPEED_COUNTER_ROLLOVER
#define REPORT_HIGH_MEAN
//#define RUN_TIME_TEST
#define RUN_LONG_TIME_TEST

// We have 13 bits. The counter may wrap (Actually max wind counts +1)
//#define MAX_WIND_CTS     0b1111111111111
#define MAX_WIND_CTS     8192

// Every 5 seconds. For 120 seconds we need a record at each end so 120 /5 +1
#define NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP   25

/*
 *
 */
class SW_Wind_Speed_Mean: public SW_MCP2318_GPIO_Sensor
{
public:
	// Raw counts
	int WindSpeedQueue[NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP];
	int CurrentSpeedQueueLoc;
	//byte SpeedQueueLength; // data chunks not bytes
	bool HaveFullSpeedQueue;

	SW_Wind_Speed_Mean(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);

	bool AcquireData();
	int GetMostRecentRawMean();
	void SendMostRecentRawMean();

#ifdef RUN_TIME_TEST
	unsigned long SW_Wind_Time_holder[NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP];
#endif /*RUN_TIME_TEST*/
#ifdef RUN_LONG_TIME_TEST
	unsigned long SW_Wind_Time_LAST;
	unsigned long SW_Wind_Time_CURRENT;
	int SW_Wind_Time_COUNT;
#endif /*RUN_LONG_TIME_TEST*/
};

#endif /* SW_WIND_SPEED_MEAN_H_ */
