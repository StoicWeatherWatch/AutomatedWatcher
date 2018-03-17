/*
 * SW_ChipCap2_Sensor.h
 *
 *  Created on: 2018-03-16
 *      Author: StoicWeather
 *
 *      There is a 10 ms window in which to start programming, Start_CM, from power up.
 *
 *      Any command that does not require data must be zero padded with two bytes of zeros.
 */

#ifndef SW_CHIPCAP2_SENSOR_H_
#define SW_CHIPCAP2_SENSOR_H_

#include "SWSensor.h"
#include "SW_Helper_Functions.h"

#define CHIPCAP2_STRT_CM_CMD     0xA0
#define CHIPCAP2_STRT_NOM_CMD    0x80

#define CHIPCAP2_CONFIG_READ_REG       0x1C
#define CHIPCAP2_CONFIG_WRITE_REG      0x5C

#define CHIPCAP2_CONFIG_VALUE      0x0028

// The response of any command except EPROM fetch is status [7:6] Diagnostics [5:2] Response [1:0]
#define CHIPCAP2_STRT_CM_RESPONSE_1_VALUE      0b00000001
#define CHIPCAP2_STRT_CM_RESPONSE_2_VALUE      0x00

/*
 *
 */
class SW_ChipCap2_Sensor: public SW_Sensor
{
public:
	bool SensorInitialized;


	SW_ChipCap2_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);

	bool InitializeSensor();
	void ReportInitialization();

	void SendMeasurmentRequest();
	void PerformDataFetch();

#ifdef VERIFY_CHIPS
	byte RevisionNum;
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/
};

#endif /* SW_CHIPCAP2_SENSOR_H_ */
