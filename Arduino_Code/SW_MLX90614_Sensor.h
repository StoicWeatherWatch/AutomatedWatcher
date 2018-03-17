/*
 * SW_MLX90614_Sensor.h
 *
 *  Created on: 2018-03-16
 *      Author: StoicWeather
 *
 *      Be careful where you write. Certain bits will destroy the calibration.
 *      Write zeros before writing values.
 */

#ifndef SW_MLX90614_SENSOR_H_
#define SW_MLX90614_SENSOR_H_

#include "SWSensor.h"
#include "SW_Helper_Functions.h"

//#define MLX90614_SEND_OTHER_DATA_IN_COMMENT

#define MLX90614_CONFIG_EEPROM_REG  0b00100101
#define MLX90614_ID_EEPROM_REG      0x3C

#define MLX90614_TA_RAM_REG       0x06
#define MLX90614_TO1_RAM_REG      0x07
#define MLX90614_TO2_RAM_REG      0x08

#define MLX90614_ID_EEPROM_MSB_VALUE        0x0B
#define MLX90614_CONFIG_EEPROM_MSB_VALUE    0xF4
#define MLX90614_CONFIG_EEPROM_LSB_VALUE    0xAF

/*
 *
 */
class SW_MLX90614_Sensor: public SW_Sensor
{
public:
	SW_MLX90614_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);

	bool InitializeSensor();

	void GetTO1DataAndSend();

#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/
};

#endif /* SW_MLX90614_SENSOR_H_ */
