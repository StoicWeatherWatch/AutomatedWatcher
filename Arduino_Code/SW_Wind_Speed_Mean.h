/*
 * SW_Wind_Speed_Mean.h
 *
 *  Created on: 2018-04-07
 *      Author: StoicWeather
 */

#ifndef SW_WIND_SPEED_MEAN_H_
#define SW_WIND_SPEED_MEAN_H_

#include "SW_MCP2318_GPIO_Sensor.h"



//#define TEST_REPORT_MEAN_STATUS

// We have 13 bits. The counter may wrap
#define MAX_WIND_CTS     0b1111111111111

#define NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP   24

/*
 *
 */
class SW_Wind_Speed_Mean: public SW_MCP2318_GPIO_Sensor
{
public:
	// Raw counts
	byte WindSpeedQueue[NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP*2];
	int CurrentSpeedQueueLoc;
	//byte SpeedQueueLength; // data chunks not bytes
	bool HaveFullSpeedQueue;

	SW_Wind_Speed_Mean(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);

	bool AcquireData();
	int GetMostRecentRawMean();
	void SendMostRecentRawMean();
};

#endif /* SW_WIND_SPEED_MEAN_H_ */
