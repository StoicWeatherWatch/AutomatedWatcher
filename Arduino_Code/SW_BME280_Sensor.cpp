/*
 * SW_BME280_Sensor.cpp
 *
 *  Created on: 2018-02-06
 *      Author: StoicWeather
 */

#include "SW_BME280_Sensor.h"

SW_BME280_Sensor::SW_BME280_Sensor(byte AddressIn,I2C I2CBussIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{


}

bool SW_BME280_Sensor::InitializeSensor()
{
	Serial.println(F("#Initialize BME280;"));

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
		Serial.println(F("#BME280 Mode Error;"));
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
bool SW_BME280_Sensor::RetrieveDataAndSend()
{
	I2CBuss.read(SensorAddress, (byte)BME280_DATASTART_REG, (byte)BME280_DATA_LEN, DataRaw);

	Serial.print(F("*"));
	Serial.print(SensorNumber,DEC);
	Serial.print(F("TPH,"));

	for(int i = 0; i < BME280_DATA_LEN; i++)
	{
		SerialHexBytePrint(DataRaw[i]);
		//Serial.print(DataRaw[i],HEX);
		if((i == 2) || (i == 5))
		{
			Serial.print(F(","));
		}
	}

	Serial.print(F(",^"));

	// TODO Put SEC-DED code here

	Serial.println(F(";"));

	// TEST Lines
	/*Serial.print("#");
	Serial.print(SensorNumber,DEC);
	Serial.print("TPH,");
	for(int i = 0; i < BME280_DATA_LEN; i++)
		{
			SerialHexBytePrint(DataRaw[i]);
			//Serial.print(DataRaw[i],HEX);
			if((i == 2) || (i == 5))
			{
				Serial.print(F(","));
			}
		}
	Serial.println(F(";"));*/
// END TEST

	return true;
}


// Verify that the chip we are talking to is correct.
#ifdef VERIFY_CHIPS
	bool SW_BME280_Sensor::VerifyChip()
	{
		// Get Sensor ID and check it
		//byte status = I2CBuss.read(SensorAddress, (byte)BME280_CHIPID_REG, (byte)1);
		I2CBuss.read(SensorAddress, (byte)BME280_CHIPID_REG, (byte)1);

		int DataIn = (int)I2CBuss.receive();


		if(DataIn == (int)BME280_CHIPID_VALUE)
		{
			Serial.println(F("#BME280 Correct Chip ID;"));
		}
		else
		{
			Serial.println(F("#NOT BME280 Correct Chip ID;"));
			return false;
		}



		return true;
	}
#endif /*VERIFY_CHIPS*/

#ifdef SEND_CALIBRATION_DATA
	bool SW_BME280_Sensor::SendCalibrationData()
	{
		byte CalData[26];

		// It appears there is a limit on the amount of data that can be read out.
		// This may be with I2C or the BME280 or both. Using the line below results in
		// zeros for the first 9 bytes. Oddly there are issues with other reads too.
		// 17 byte limit?
		//I2CBuss.read(SensorAddress, (byte)BME280_CAL2_BLK_REG, (byte)BME280_CAL2_BLK_LEN, CalData);
		//I2CBuss.read(SensorAddress, (byte)BME280_CAL2_BLK_REG, (byte)BME280_CAL2_BLK_LEN, CalData);

		Serial.println(F("#BME280 Calibration Data Block 2;"));


		for(int i = 0; i < BME280_CAL2_BLK_LEN; i++)
		{
			I2CBuss.read(SensorAddress, (byte)BME280_CAL2_BLK_REG+i, (byte)1, CalData+i);
			Serial.print(F("!"));
			Serial.print(CALIBRATION_DATA_KEYWORD_BLK2);
			Serial.print(i);
			Serial.print(F(","));
			SerialHexBytePrint(CalData[i]);
			Serial.print(F("#"));
			SerialHexBytePrint((byte)BME280_CAL2_BLK_REG +i);
			Serial.println(F(";"));


		}

		//TEST LINES
		/*Serial.print("# A ");
		Serial.print(BME280_CAL2_BLK_REG,HEX);
		Serial.print("  ");

		I2CBuss.read((byte)SensorAddress,(byte)BME280_CAL2_BLK_REG, (byte)2);

		Serial.print((byte)I2c.receive(),HEX);
		Serial.print((byte)I2c.receive(),HEX);

		Serial.println(F(";"));

		Serial.print("# B ");
		Serial.print(BME280_CAL2_BLK_REG,HEX);
		Serial.print("  ");

		I2CBuss.read(SensorAddress, ((byte)BME280_CAL2_BLK_REG), (byte)2, CalData);

		Serial.print(CalData[0],HEX);
		Serial.print(CalData[1],HEX);

		Serial.println(F(";"));

		for(int i = 0; i < 26; i++)
		{
			I2CBuss.read(SensorAddress, ((byte)BME280_CAL2_BLK_REG + i), (byte)2, CalData);
			Serial.print(F("Error code "));

			Serial.println(((byte)BME280_CAL2_BLK_REG + i),HEX);
			Serial.print(CalData[0],HEX);
			Serial.println(CalData[1],HEX);

		}

*/
		// END TEST LINES

		I2CBuss.read(SensorAddress, (byte)BME280_CAL1_BLK_REG, (byte)BME280_CAL1_BLK_LEN, CalData);

		Serial.println(F("#BME280 Calibration Data Block 1;"));
		for(int i = 0; i < BME280_CAL1_BLK_LEN; i++)
		{

			Serial.print(F("!"));
			Serial.print(CALIBRATION_DATA_KEYWORD_BLK1);
			Serial.print(i);
			Serial.print(F(","));
			SerialHexBytePrint(CalData[i]);
			Serial.print(F("#"));
			SerialHexBytePrint((byte)BME280_CAL1_BLK_REG +i);
			Serial.println(F(";"));
		}


		return true;
	}
#endif /*SEND_CALIBRATION_DATA*/



