/*
 * SW_Wind_Gust.h
 *
 *  Created on: 2018-02-10
 *      Author: StoicWeather
 */

#ifndef SW_WIND_GUST_H_
#define SW_WIND_GUST_H_

#include "SW_MCP2318_GPIO_Sensor.h"
#include "SW_Wind_Dir_Analog.h"

// This assumes 1 count = 1 MPH. True if reading out every 2.25 seconds. 10 knots = 11.5078 MPH. We reound up.
#define GUST_MINIMUM_DIFFERENCE 12


// For testing, 12 is hard to hit rotating by hand
//#define GUST_MINIMUM_DIFFERENCE 3
//#define RUN_GUST_SIMULATION

// We have 13 bits. The counter may wrap
#define MAX_WIND_CTS     0b1111111111111

//#define TEST_PRINTS

/*
 *
 */
class SW_Wind_Gust: public SW_MCP2318_GPIO_Sensor, public SW_Wind_Dir_Analog
{
public:
	// Unlike WindSpeedQueue, GustQueue holds differences between each record. This saves memory
		// since we only need a byte not 13 bits.
	byte *WindSpeedGustQueue;
		int CurrentSpeedQueueLoc;
		byte SpeedQueueLength; // data chunks not bytes
		bool HaveFullSpeedQueue;

		int LastGustReadout;

	// This holds a record of if a gust was sent in each bit. 0 for unsent 1 for sent
	byte* SentRecordQueue;



	SW_Wind_Gust(byte MCP23018AddressIn, I2C I2CBussIn, byte NumberofGustSpeedRecordsIn, byte SpeedSensorNumberIN, byte AAQ0PinIn, byte NumberOfDirectionRecordsIn, byte DirectionSensorNumberIn);

	void AcquireWindGustDirection();
	void AcquireWindGustSpeed();
	void SendWindGustData();

	bool GustNotSentPreviously(byte QueueLocGust);
	void SetSentRecord(byte GustSpeedQueueLocToSet,bool HasBeenSent);


};

#endif /* SW_WIND_GUST_H_ */
