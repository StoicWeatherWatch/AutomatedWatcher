/*
 * SW_BME280_Sensor.h
 *
 *  Created on: 2018-01-21
 *      Author: StoicWeather
 */

#ifndef SW_BME280_SENSOR_H_
#define SW_BME280_SENSOR_H_

#include "SWSensor.h"

#define BME280_CHIPID_REG     0xD0
#define BME280_RESET_REG      0xE0
#define BME280_CONFIG_REG     0xF5
#define BME280_CTLHUM_REG     0xF2
#define BME280_CTLMESR_REG    0xF4
#define BME280_DATASTART_REG  0xF7
#define BME280_DATA_LEN         8


#define BME280_RESET_CMD      0xB6
#define BME280_CONFIG_CMD     0xF4
#define BME280_CTLHUM_CMD     0x03
#define BME280_CTLMESR_CMD    0x6E


#define BME280_CHIPID_VALUE   0x60
#define BME280_MODE_MASK      0x03


#ifdef SEND_CALIBRATION_DATA
#define BME280_CAL1_BLK_REG   0xE1
#define BME280_CAL1_BLK_LEN    15
#define BME280_CAL2_BLK_REG   0x88
#define BME280_CAL2_BLK_LEN    25

#define CALIBRATION_DATA_KEYWORD_BLK1 "BME280-A-CAL1."
#define CALIBRATION_DATA_KEYWORD_BLK2 "BME280-A-CAL2."
// bytes
#endif /*SEND_CALIBRATION_DATA*/


/*
 *
 */
class SW_BME280_Sensor: public SW_Sensor
{
	protected:
	byte DataRaw[BME280_DATA_LEN];


public:
	SW_BME280_Sensor(byte AddressIn, I2C I2CBussIn);
	bool InitializeSensor();
	bool AcquireData();
	// First call AcquireData then wait minimum per data sheet
	bool RetrieveData();

#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/

#ifdef SEND_CALIBRATION_DATA
	bool SendCalibrationData();
#endif /*SEND_CALIBRATION_DATA*/
};

#endif /* SW_BME280_SENSOR_H_ */

/*
 * Notes on configuration
 * Per datasheet - recommendations for weather monitoring
 * Forced mode at 1 sample / min
 * Oversampleing 1
 * IIR off
 * Page 16 - Looks like an IIR setting of 4 will work for us
 * Page 19 Try oversampleing everything 4
 *
 * To ctrl-hum 011 to oversample 4 in full 0000 0011 0x03
 * Change ctrl-hum first then ctrl-meas
 * For T over sample 4 use 011
 * For P over sample 4 use 011
 * For forced mode 10
 * 0110 1110 0x6E
 *
 * Need to set config in sleep mode before setting other ctrl
 * 111
 * IIR 4 is 101
 * Very much 00 for SPI off
 * 1111 0100  0xF4
 *
 *
 */
