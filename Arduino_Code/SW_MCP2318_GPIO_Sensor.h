/*
 * SW_MCP2318_GPIO_Sensor.h
 *
 *  Created on: 2018-02-01
 *      Author: StoicWeather
 */
/*
 * NWS Wind gust rapid fluctuations of wind speed with a variation of 10 knots or more between peaks and lulls.
 * The gust speed is the max speed recorded. 10 knots is 11.5 clicks in 2.25 seconds.
 *
 * Data conventions
 * W for wind speed
 * WA for all wind (The entire mean queue)
 * WM for a raw mean. Devide by sample period to get actual mean
 * WG for gust in raw Current, min, Max
 *
 */

#ifndef SW_MCP2318_GPIO_SENSOR_H_
#define SW_MCP2318_GPIO_SENSOR_H_

// These assume BANK = 0
#define MCP2318_IOCON_REG    0x0A
#define MCP2318_IODIRA_REG   0x00
#define MCP2318_GPIOA_REG    0x12
#define MCP2318_GPIOB_REG    0x13


#define MCP2318_IOCON_DEFAULT_VALUE 	 	0x0000
#define MCP2318_IODIRA_DEFAULT_VALUE  	0xFFFF

//Bank  = 0 MIRROR = 0 SEQOP = 0
// SEQOP may cause issues
#define MCP2318_CONFIG_IOCON_CMD    0x00

// 2.25 seconds.
#define NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP   54

#define MAX_WIND_CTS     0b1111111111111

#include "SWSensor.h"
#include "SW_Helper_Functions.h"

/*
 *
 */
class SW_MCP2318_GPIO_Sensor: public SW_Sensor {
public:
	// Raw counts
	byte *WindSpeedQueue;
	int CurrentQueueLoc;
	byte QueueLength; // data chunks not bytes
	bool HaveFullQueue;


	// Unlike WindSpeedQueue, GustQueue holds differences between each record. This saves memory
	// since we only need a byte not 13 bits.
	byte *GustQueue;
	int LastGustReadout;
	bool HaveFullGustQueue;
	int CurrentGustQueueLoc;



	SW_MCP2318_GPIO_Sensor(byte AddressIn, I2C I2CBussIn, byte NumberOfRecordsIn, byte SensorNumberIN);
	bool AcquireData(); // For mean wind speed
	bool SendAllRawDataSerial(); // For mean wind speed
	bool SendRawDataSerial();

	int AcquireDataAndReturn();

	int GetMostRecentRawMean();
	bool SendMostRecentRawMean();


	// A gust is 10 knts between peak and lull in a time period that is rapid.
	//Readout is every 2.25 seconds for instantaneous speed.
	// How many 2.25 second intervals is set by gust Record Interval
	//bool AcquireGustData();
	// THis sends raw counts from a 2.25 second period. Sends max, min, and current for
	// Time period recorded.
	//bool SendGustData();
	bool AcquireGustDataAndSend();


	bool InitializeSensor();



#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/
};

#endif /* SW_MCP2318_GPIO_SENSOR_H_ */
