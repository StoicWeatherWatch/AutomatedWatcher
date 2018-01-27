/*
 * SW_MCP9808_Sensor.cpp
 *
 *  Created on: 2018-01-21
 *      Author: StoicWeather
 */

#include "SW_MCP9808_Sensor.h"

SW_MCP9808_Sensor::SW_MCP9808_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{


}

bool SW_MCP9808_Sensor::InitializeSensor()
{

	Serial.println("#Initializing MCP9808;");

#ifdef VERIFY_CHIPS
	if(!VerifyChip())
	{
		return false;
	}
#endif /*VERIFY_CHIPS*/

	// Set for maximum temperature resolution
	I2CBuss.write(SensorAddress, (byte)MCP9808_RES_REG, (byte)MCP9808_MAXRES_CMD);

	byte Configuration[2];
	Configuration[0] = (byte)MCP9808_CONFIG_MSB_CMD;
	Configuration[1] = (byte)MCP9808_CONFIG_LSB_CMD;
	I2CBuss.write(SensorAddress, (byte)MCP9808_CONFIG_REG, Configuration, (byte)2);


	return true;
}

/*
 * This will read data into the data variable. It returns true on success. A false return is less certain.
 */
bool SW_MCP9808_Sensor::AcquireData()
{
	I2CBuss.read(SensorAddress, (byte)MCP9808_TEMP_REG, (byte)2, TemperatureRaw);

	//Serial.println(TemperatureRaw[0],HEX);
	//Serial.println(TemperatureRaw[1],HEX);


	return true;
}

bool SW_MCP9808_Sensor::SendRawDataSerial()
{
	Serial.print("*");
	Serial.print(SensorNumber,DEC);
	Serial.print("T,");
	SerialHexBytePrint(TemperatureRaw[0]);
	SerialHexBytePrint(TemperatureRaw[1]);
	//Serial.print(TemperatureRaw[0],HEX);
	//Serial.print(TemperatureRaw[1],HEX);
	Serial.println(";");

	return true;
}

byte SW_MCP9808_Sensor::GetRawTempreature_HighBits()
{
	return TemperatureRaw[0];
}
byte SW_MCP9808_Sensor::GetRawTempreature_LowBits()
{
	return TemperatureRaw[1];
}

#ifdef PERFORM_DATA_PROCESSING
	float SW_MCP9808_Sensor::ProcessTemp()
	{
		float T = 0;

		// TODO choose a method

		//Method 1
		int shift = 3;
		for (int i = 7; i >= 4; i--)
		{
			 T += pow(2.0,(float)i) * ((TemperatureRaw[0] >> shift) & (byte)1);
			 shift--;
		}
		shift = 7;
		for (int i = 3; i >= -4; i--)
		{
			T += pow(2.0,(float)i) * ((TemperatureRaw[1] >> shift) & (byte)1);
			shift--;
		}

		// Negative if true
		if((TemperatureRaw[0] >> 4) & (byte)1)
		{
			T = -1.0*T;
		}

		//Method 2
		if(!((TemperatureRaw[0] >> 4) & (byte)1))
		{
			T = (float)(TemperatureRaw[0] & (byte)0x0F) * pow(2.0,4.0);
			T += (float)(TemperatureRaw[1]) * pow(2.0,-4.0);

		}
		else
		{
			T = 256.0 - (float)(TemperatureRaw[0] & (byte)0x0F) * pow(2.0,4.0);
			T = T - (float)(TemperatureRaw[1]) * pow(2.0,-4.0);
		}



		return T;

	}
#endif /*PERFORM_DATA_PROCESSING*/


// Verify that the chip we are talking to is correct.
#ifdef VERIFY_CHIPS
	bool SW_MCP9808_Sensor::VerifyChip()
	{
		// Get Sensor ID and check it
		byte status = I2CBuss.read(SensorAddress, (byte)MCP9808_MANID_REG, (byte)2);

		int DataIn = (int)I2CBuss.receive();
		// Most sig bits came in first. They get shifted up.
		DataIn <<= 8;
		DataIn |= (int)I2CBuss.receive();

		//Serial.println(status);
		//Serial.println(DataIn,HEX);

		if(DataIn == (int)MCP9808_MANID_VALUE)
		{
			Serial.println("#MCP9808 Correct Man ID;");
		}
		else
		{
			Serial.println("#NOT MCP9808 Correct Man ID;");
			return false;
		}

		status = I2CBuss.read(SensorAddress, (byte)MCP9808_CHIPID_REG, (byte)2);

		DataIn = (int)I2CBuss.receive();
		// Most sig bits came in first. They get shifted up.
		DataIn <<= 8;
		DataIn |= (int)I2CBuss.receive();

		//Serial.println(status);
		//Serial.println(DataIn,HEX);

		if(DataIn == (int)MCP9808_CHIPID_VALUE)
		{
			Serial.println("#Correct Chip;");
		}
		else
		{
			Serial.println("#NOT Correct Chip;");
			return false;
		}


		return true;
	}
#endif /*VERIFY_CHIPS*/
