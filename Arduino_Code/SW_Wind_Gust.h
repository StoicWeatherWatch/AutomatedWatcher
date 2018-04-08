/*
 * SW_Wind_Gust.h
 *
 *  Created on: 2018-04-07
 *      Author: StoicWeather
 */

// TODO Seems to spit garbage when unconnected. Getting *6WGS,FA;+5WGD,267; even when no wind I2C connected

#ifndef SW_WIND_GUST_H_
#define SW_WIND_GUST_H_

#include "SW_MCP2318_GPIO_Sensor.h"
#include "SW_Wind_Dir_Analog.h"

// This assumes 1 count = 1 MPH. True if reading out every 2.25 seconds. 10 knots = 11.5078 MPH. We round up.
#define GUST_MINIMUM_DIFFERENCE 12


// For testing, 12 is hard to hit rotating by hand
//#define GUST_MINIMUM_DIFFERENCE 3
//#define RUN_GUST_SIMULATION

// We have 13 bits. The counter may wrap
#define MAX_WIND_CTS     0b1111111111111

// Direction and speed
#define NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP   54
// This is NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP /8 and rounded up
#define NUMBER_OF_BYTES_NEEDED_FOR_SENT_RECORD_QUEUE   7
// This is NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP * 3/2 (An even number is assumed. Otherwise all hell may break loose.)
#define NUMBER_OF_BYTES_NEEDED_FOR_DIRECTION_QUEUE   81

//#define TEST_PRINTS
//#define TEST_REPORT_STATUS
#define REPORT_GUST_COUNTER_ROLLOVER

/*
 *
 */
class SW_Wind_Gust: public SW_MCP2318_GPIO_Sensor, public SW_Wind_Dir_Analog
{
public:
	// Unlike WindSpeedQueue, GustQueue holds differences between each record. This saves memory
		// since we only need a byte not 13 bits.
		byte WindSpeedGustQueue[NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP];
		int CurrentSpeedQueueLoc;
		//byte SpeedQueueLength; // data chunks not bytes
		bool HaveFullSpeedQueue;
		int LastGustReadout;

		/* This is used to hold direction of gust. It is fed to SW_Wind_Dir_Analog
		 * It is created here to avoid a need for calloc since other subclasses of SW_Ard_Analog use different lengths.
		 * */
		byte GustDirectionQueue[NUMBER_OF_BYTES_NEEDED_FOR_DIRECTION_QUEUE];

	// This holds a record of if a gust was sent in each bit. 0 for unsent 1 for sent
	byte SentRecordQueue[NUMBER_OF_BYTES_NEEDED_FOR_SENT_RECORD_QUEUE];



	SW_Wind_Gust(byte MCP23018AddressIn, I2C I2CBussIn, byte SpeedSensorNumberIN, byte AAQ0PinIn, byte DirectionSensorNumberIn);

	void AcquireWindGustDirection();
	void AcquireWindGustSpeed();
	void SendWindGustData();

	bool GustNotSentPreviously(byte QueueLocGust);
	void SetSentRecord(byte GustSpeedQueueLocToSet,bool HasBeenSent);


};

#endif /* SW_WIND_GUST_H_ */
