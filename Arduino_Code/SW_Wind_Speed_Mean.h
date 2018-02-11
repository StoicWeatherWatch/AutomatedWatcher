/*
 * SW_Wind_Speed_Mean.h
 *
 *  Created on: 2018-02-06
 *      Author: StoicWeather
 */

#ifndef SW_WIND_SPEED_MEAN_H_
#define SW_WIND_SPEED_MEAN_H_

#include "SW_MCP2318_GPIO_Sensor.h"

// We have 13 bits. The counter may wrap
#define MAX_WIND_CTS     0b1111111111111

/*
 *
 */
class SW_Wind_Speed_Mean: public SW_MCP2318_GPIO_Sensor
{
public:
	// Raw counts
	byte *WindSpeedQueue;
	int CurrentSpeedQueueLoc;
	byte SpeedQueueLength; // data chunks not bytes
	bool HaveFullSpeedQueue;

	SW_Wind_Speed_Mean(byte AddressIn, I2C I2CBussIn, byte NumberOfSpeedRecordsIn, byte SensorNumberIN);

	bool AcquireData();
	int GetMostRecentRawMean();
	void SendMostRecentRawMean();
};

#endif /* SW_WIND_SPEED_MEAN_H_ */
