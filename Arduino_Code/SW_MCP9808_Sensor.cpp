/*
 * SW_MCP9808_Sensor.cpp
 *
 *  Created on: 2018-01-21
 *      Author: StoicWeather
 */

#include "SW_MCP9808_Sensor.h"

SW_MCP9808_Sensor::SW_MCP9808_Sensor(byte AddressIn, I2C I2CBussIn)
:SW_Sensor(AddressIn,I2CBussIn)
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





	return true;
}

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
			Serial.println("#MCP9808 Correct Man ID");
		}
		else
		{
			Serial.println("#NOT MCP9808 Correct Man ID");
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
			Serial.println("#Correct Chip");
		}
		else
		{
			Serial.println("#NOT Correct Chip");
			return false;
		}


		return true;
	}
#endif /*VERIFY_CHIPS*/
