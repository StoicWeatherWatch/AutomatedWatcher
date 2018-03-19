/*
 * SW_BMP280_Sensor.h
 *
 *  Created on: 2018-03-17
 *      Author: StoicWeather
 *
 *      Key difference with BME280 osrs_p and osrs_t set resolution
 */

#ifndef SW_BMP280_SENSOR_H_
#define SW_BMP280_SENSOR_H_

#include "SWSensor.h"
#include "SW_Helper_Functions.h"

//#define SEND_CALIBRATION_DATA

#define BMP280_CHECK_STATUS_BEFORE_SEND

#define BMP280_CHIPID_REG     0xD0
#define BMP280_RESET_REG      0xE0
#define BMP280_CONFIG_REG     0xF5
#define BMP280_CTLMESR_REG    0xF4
#define BMP280_DATASTART_REG  0xF7
#define BMP280_STATUS_REG     0xF3

#define BMP280_DATA_LEN         6


#define BMP280_RESET_CMD      0xB6
#define BMP280_CONFIG_CMD     0xF4
#define BMP280_CTLMESR_CMD    0xFE


#define BMP280_CHIPID_VALUE   0x58
#define BMP280_MODE_MASK      0x03
#define BMP280_STATUS_MASK    0x09


#ifdef SEND_CALIBRATION_DATA
//#define BMP280_CAL1_BLK_REG    0xE1
//#define BMP280_CAL1_BLK_LEN     8
#define BMP280_CAL2_BLK_REG    0x88
#define BMP280_CAL2_BLK_LEN     26

//#define CALIBRATION_DATA_KEYWORD_BLK1 "cal-BMP280-1.1."
#define CALIBRATION_DATA_KEYWORD_BLK2 "cal-BMP280-2.2."

#endif /*SEND_CALIBRATION_DATA*/

/*
 *
 */
class SW_BMP280_Sensor: public SW_Sensor
{
	protected:
	byte DataRaw[BMP280_DATA_LEN];

public:
	SW_BMP280_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);

	bool InitializeSensor();
	bool AcquireData();
	// First call AcquireData then wait minimum per data sheet
	// At 4,4,4,4 30 ms will be the max delay. Read after 30 ms. (From BME280  might be different)
	bool RetrieveDataAndSend();

#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/

#ifdef SEND_CALIBRATION_DATA
	bool SendCalibrationData();
#endif /*SEND_CALIBRATION_DATA*/
};

#endif /* SW_BMP280_SENSOR_H_ */
