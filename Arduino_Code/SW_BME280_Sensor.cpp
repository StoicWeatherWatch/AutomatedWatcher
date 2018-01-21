/*
 * SW_BME280_Sensor.cpp
 *
 *  Created on: 2018-01-21
 *      Author: StoicWeather
 */

#include "SW_BME280_Sensor.h"

SW_BME280_Sensor::SW_BME280_Sensor(byte AddressIn,I2C I2CBussIn)
:SW_Sensor(AddressIn,I2CBussIn)
{


}

bool SW_BME280_Sensor::InitializeSensor()
{
	Serial.println("Initialize BME280");

#ifdef VERIFY_CHIPS
	if(!VerifyChip())
	{
		return false;
	}
#endif /*VERIFY_CHIPS*/



	return true;
}

// Verify that the chip we are talking to is correct.
#ifdef VERIFY_CHIPS
	bool SW_BME280_Sensor::VerifyChip()
	{
		// Get Sensor ID and check it
		byte status = I2CBuss.read(SensorAddress, (byte)BME280_CHIPID_REG, (byte)1);

		int DataIn = (int)I2CBuss.receive();

		//Serial.println(status);
		//Serial.println(DataIn,HEX);

		if(DataIn == (int)BME280_CHIPID_VALUE)
		{
			Serial.println("#BME280 Correct Chip ID");
		}
		else
		{
			Serial.println("#NOT BME280 Correct Chip ID");
			return false;
		}



		return true;
	}
#endif /*VERIFY_CHIPS*/

