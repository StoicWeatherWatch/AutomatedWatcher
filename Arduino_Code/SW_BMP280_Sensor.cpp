/*
 * SW_BMP280_Sensor.cpp
 *
 *  Created on: 2018-03-17
 *      Author: StoicWeather
 */

#include "SW_BMP280_Sensor.h"

SW_BMP280_Sensor::SW_BMP280_Sensor(byte AddressIn,I2C I2CBussIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{


}


bool SW_BMP280_Sensor::InitializeSensor()
{
	Serial.println(F("#Initialize BMP280;"));

#ifdef VERIFY_CHIPS
	if(!VerifyChip())
	{
		return false;
	}
#endif /*VERIFY_CHIPS*/

	// Send reset
	I2CBuss.write(SensorAddress,(byte)BMP280_RESET_REG, (byte)BMP280_RESET_CMD);


#ifdef SEND_CALIBRATION_DATA
	SendCalibrationData();
#endif /*SEND_CALIBRATION_DATA*/

	// Verify sleep mode
	I2CBuss.read(SensorAddress, (byte)BMP280_CTLMESR_CMD, (byte)1);
	byte RegIn = I2CBuss.receive();
	if((RegIn & (byte)BMP280_MODE_MASK) >0)
	{
		Serial.println(F("#BMP280 Mode Error;"));
		return false;
	}

	// Configure the sensor
	I2CBuss.write(SensorAddress,(byte)BMP280_CONFIG_REG, (byte)BMP280_CONFIG_CMD);
	I2CBuss.write(SensorAddress,(byte)BMP280_CTLMESR_REG, (byte)BMP280_CTLMESR_CMD);


	return true;
}

// To get data we need to write the forced mode command to ctrl-meas
bool SW_BMP280_Sensor::AcquireData()
{
	I2CBuss.write(SensorAddress,(byte)BMP280_CTLMESR_REG, (byte)BMP280_CTLMESR_CMD);

	return true;
}

// First call AcquireData then wait time minimum per data sheet
bool SW_BMP280_Sensor::RetrieveDataAndSend()
{
#ifdef BMP280_CHECK_STATUS_BEFORE_SEND
	// If still measuring or not copied, do not send
	byte StatusReg = 0;
	I2CBuss.read(SensorAddress, (byte)BMP280_STATUS_REG, (byte)1);
	StatusReg = I2CBuss.receive();

	if((StatusReg & BMP280_STATUS_MASK) != (byte)0)
	{
		Serial.println(F("#BMP280 Attempted to readout, sensor not ready;"));
		return false;
	}
#endif /*BMP280_CHECK_STATUS_BEFORE_SEND*/



	I2CBuss.read(SensorAddress, (byte)BMP280_DATASTART_REG, (byte)BMP280_DATA_LEN, DataRaw);

	Serial.print(F("*"));
	Serial.print(SensorNumber,DEC);
	Serial.print(F("TP,"));

	for(int i = 0; i < BMP280_DATA_LEN; i++)
	{
		SerialHexBytePrint(DataRaw[i]);
		//Serial.print(DataRaw[i],HEX);
		if((i == 2))
		{
			Serial.print(F(","));
		}
	}

	Serial.print(F(",^"));

	// TODO Put SEC-DED code here

	Serial.println(F(";"));



	return true;
}//RetrieveDataAndSend()


// Verify that the chip we are talking to is correct.
#ifdef VERIFY_CHIPS
	bool SW_BMP280_Sensor::VerifyChip()
	{
		// Get Sensor ID and check it
		//byte status = I2CBuss.read(SensorAddress, (byte)BMP280_CHIPID_REG, (byte)1);
		I2CBuss.read(SensorAddress, (byte)BMP280_CHIPID_REG, (byte)1);

		int DataIn = (int)I2CBuss.receive();


		if(DataIn == (int)BMP280_CHIPID_VALUE)
		{
			Serial.println(F("#BMP280 Correct Chip ID;"));
		}
		else
		{
			Serial.println(F("#NOT BMP280 Correct Chip ID;"));
			return false;
		}



		return true;
	}
#endif /*VERIFY_CHIPS*/

#ifdef SEND_CALIBRATION_DATA
	bool SW_BMP280_Sensor::SendCalibrationData()
	{
		byte CalData[26];

		// It appears there is a limit on the amount of data that can be read out.
		// This may be with I2C or the BMP280 or both. Using the line below results in
		// zeros for the first 9 bytes. Oddly there are issues with other reads too.
		// 17 byte limit?
		//I2CBuss.read(SensorAddress, (byte)BMP280_CAL2_BLK_REG, (byte)BMP280_CAL2_BLK_LEN, CalData);
		//I2CBuss.read(SensorAddress, (byte)BMP280_CAL2_BLK_REG, (byte)BMP280_CAL2_BLK_LEN, CalData);

		Serial.println(F("#BMP280 Calibration Data Block 2;"));


		for(int i = 0; i < BMP280_CAL2_BLK_LEN; i++)
		{
			I2CBuss.read(SensorAddress, (byte)BMP280_CAL2_BLK_REG+i, (byte)1, CalData+i);
			Serial.print(F("!"));
			Serial.print(CALIBRATION_DATA_KEYWORD_BLK2);
			Serial.print(i);
			Serial.print(F(","));
			SerialHexBytePrint(CalData[i]);
			Serial.print(F("#"));
			SerialHexBytePrint((byte)BMP280_CAL2_BLK_REG +i);
			Serial.println(F(";"));


		}

		//TEST LINES
		/*Serial.print("# A ");
		Serial.print(BMP280_CAL2_BLK_REG,HEX);
		Serial.print("  ");

		I2CBuss.read((byte)SensorAddress,(byte)BMP280_CAL2_BLK_REG, (byte)2);

		Serial.print((byte)I2c.receive(),HEX);
		Serial.print((byte)I2c.receive(),HEX);

		Serial.println(F(";"));

		Serial.print("# B ");
		Serial.print(BMP280_CAL2_BLK_REG,HEX);
		Serial.print("  ");

		I2CBuss.read(SensorAddress, ((byte)BMP280_CAL2_BLK_REG), (byte)2, CalData);

		Serial.print(CalData[0],HEX);
		Serial.print(CalData[1],HEX);

		Serial.println(F(";"));

		for(int i = 0; i < 26; i++)
		{
			I2CBuss.read(SensorAddress, ((byte)BMP280_CAL2_BLK_REG + i), (byte)2, CalData);
			Serial.print(F("Error code "));

			Serial.println(((byte)BMP280_CAL2_BLK_REG + i),HEX);
			Serial.print(CalData[0],HEX);
			Serial.println(CalData[1],HEX);

		}

*/
		// END TEST LINES

		/*No Block 1 in BMP280 only BME280
		 *
		 * I2CBuss.read(SensorAddress, (byte)BMP280_CAL1_BLK_REG, (byte)BMP280_CAL1_BLK_LEN, CalData);

		Serial.println(F("#BMP280 Calibration Data Block 1;"));
		for(int i = 0; i < BMP280_CAL1_BLK_LEN; i++)
		{

			Serial.print(F("!"));
			Serial.print(CALIBRATION_DATA_KEYWORD_BLK1);
			Serial.print(i);
			Serial.print(F(","));
			SerialHexBytePrint(CalData[i]);
			Serial.print(F("#"));
			SerialHexBytePrint((byte)BMP280_CAL1_BLK_REG +i);
			Serial.println(F(";"));
		}*/


		return true;
	}
#endif /*SEND_CALIBRATION_DATA*/



