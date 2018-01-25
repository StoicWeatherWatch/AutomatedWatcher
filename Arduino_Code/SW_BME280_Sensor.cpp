/*
 * SW_BME280_Sensor.cpp
 *
 *  Created on: 2018-01-21
 *      Author: StoicWeather
 */

#include "SW_BME280_Sensor.h"

SW_BME280_Sensor::SW_BME280_Sensor(byte AddressIn,I2C I2CBussIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{


}

bool SW_BME280_Sensor::InitializeSensor()
{
	Serial.println("#Initialize BME280;");

#ifdef VERIFY_CHIPS
	if(!VerifyChip())
	{
		return false;
	}
#endif /*VERIFY_CHIPS*/

	// Send reset
	I2CBuss.write(SensorAddress,(byte)BME280_RESET_REG, (byte)BME280_RESET_CMD);


#ifdef SEND_CALIBRATION_DATA
	SendCalibrationData();
#endif /*SEND_CALIBRATION_DATA*/

	// Verify sleep mode
	I2CBuss.read(SensorAddress, (byte)BME280_CTLMESR_CMD, (byte)1);
	byte RegIn = I2CBuss.receive();
	if((RegIn & (byte)BME280_MODE_MASK) >0)
	{
		Serial.println("#BME280 Mode Error;");
		return false;
	}

	// Configure the sensor
	I2CBuss.write(SensorAddress,(byte)BME280_CONFIG_REG, (byte)BME280_CONFIG_CMD);
	I2CBuss.write(SensorAddress,(byte)BME280_CTLHUM_REG, (byte)BME280_CTLHUM_CMD);
	I2CBuss.write(SensorAddress,(byte)BME280_CTLMESR_REG, (byte)BME280_CTLMESR_CMD);

	//I2CBuss.read(SensorAddress, (byte)BME280_CTLMESR_REG, (byte)1);
	//RegIn = I2CBuss.receive();
	//Serial.println(RegIn,HEX);

	return true;
}

// To get data we need to write the forced mode command to ctrl-meas
bool SW_BME280_Sensor::AcquireData()
{
	I2CBuss.write(SensorAddress,(byte)BME280_CTLMESR_REG, (byte)BME280_CTLMESR_CMD);

	return true;
}

// First call AcquireData then wait time minimum per data sheet
bool SW_BME280_Sensor::RetrieveData()
{
	I2CBuss.read(SensorAddress, (byte)BME280_DATASTART_REG, (byte)BME280_DATA_LEN, DataRaw);

	Serial.print("*");
	Serial.print(SensorNumber,DEC);
	Serial.print("TPH,");

	for(int i = 0; i < BME280_DATA_LEN; i++)
	{
		Serial.print(DataRaw[i],HEX);
		if((i == 2) || (i == 5))
		{
			Serial.print(",");
		}
	}

	Serial.print(",^");

	// TODO Put SEC-DED code here

	Serial.println(";");



	return true;
}

bool SW_BME280_Sensor::SendRawDataSerial()
{

	// TODO Fix this
	return true;
}


// Verify that the chip we are talking to is correct.
#ifdef VERIFY_CHIPS
	bool SW_BME280_Sensor::VerifyChip()
	{
		// Get Sensor ID and check it
		byte status = I2CBuss.read(SensorAddress, (byte)BME280_CHIPID_REG, (byte)1);

		int DataIn = (int)I2CBuss.receive();


		if(DataIn == (int)BME280_CHIPID_VALUE)
		{
			Serial.println("#BME280 Correct Chip ID;");
		}
		else
		{
			Serial.println("#NOT BME280 Correct Chip ID;");
			return false;
		}



		return true;
	}
#endif /*VERIFY_CHIPS*/

#ifdef SEND_CALIBRATION_DATA
	bool SW_BME280_Sensor::SendCalibrationData()
	{
		byte CalData[25];

		I2CBuss.read(SensorAddress, (byte)BME280_CAL2_BLK_REG, (byte)BME280_CAL2_BLK_LEN, CalData);

		Serial.println("#BME280 Calibration Data Block 2;");
		for(int i = 0; i < BME280_CAL2_BLK_LEN; i++)
		{
			Serial.print("*");
			Serial.print(CALIBRATION_DATA_KEYWORD_BLK2);
			Serial.print(i);
			Serial.print(",");
			Serial.print(CalData[i],HEX);
			Serial.println(";");

		}

		I2CBuss.read(SensorAddress, (byte)BME280_CAL1_BLK_REG, (byte)BME280_CAL1_BLK_LEN, CalData);

		Serial.println("#BME280 Calibration Data Block 1;");
		for(int i = 0; i < BME280_CAL1_BLK_LEN; i++)
		{
			Serial.print("*");
			Serial.print(CALIBRATION_DATA_KEYWORD_BLK1);
			Serial.print(i);
			Serial.print(",");
			Serial.print(CalData[i],HEX);
			Serial.println(";");

		}


		return true;
	}
#endif /*SEND_CALIBRATION_DATA*/



