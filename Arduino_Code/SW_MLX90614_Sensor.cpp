/*
 * SW_MLX90614_Sensor.cpp
 *
 *  Created on: 2018-03-16
 *      Author: StoicWeather
 */

#include "SW_MLX90614_Sensor.h"

SW_MLX90614_Sensor::SW_MLX90614_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{


}


bool SW_MLX90614_Sensor::InitializeSensor()
{

	Serial.print(F("#MLX90614 InitializeSensor number "));
	Serial.print(SensorNumber,DEC);
	Serial.println(F(";"));

#ifdef VERIFY_CHIPS
	if(!VerifyChip())
	{
		return false;
	}
#endif /*VERIFY_CHIPS*/

	return true;
}

void SW_MLX90614_Sensor::GetTO1DataAndSend()
{

	byte DataIn[2];

	// The 3rd byte is PEC checksum. If you don't read it out the whole thing blocks for ~5 sec.
	I2CBuss.read(SensorAddress, (byte)MLX90614_TO1_RAM_REG, (byte)3);

	DataIn[0] = I2CBuss.receive();
	DataIn[1] = I2CBuss.receive();
	I2CBuss.receive();

	//Very first bit of the second, MSB, is a flag. 0 is good
	if((DataIn[1] & 0x80) == 0x00)
	{
		Serial.print(F("*"));
		Serial.print(SensorNumber,DEC);
		Serial.print(F("T,"));
		SerialHexBytePrint(DataIn[1]);
		SerialHexBytePrint(DataIn[0]);
		Serial.println(F(";"));
	}
	else
	{
		Serial.print(F("# Bad Data"));
		Serial.print(SensorNumber,DEC);
		Serial.print(F("T,"));
		SerialHexBytePrint(DataIn[1]);
		SerialHexBytePrint(DataIn[0]);
		Serial.println(F(";"));
	}

#ifdef MLX90614_SEND_OTHER_DATA_IN_COMMENT

	I2CBuss.read(SensorAddress, (byte)MLX90614_TO2_RAM_REG, (byte)2);

		DataIn[0] = I2CBuss.receive();
		DataIn[1] = I2CBuss.receive();

	Serial.print(F("#MLX90614 TO2"));
			SerialHexBytePrint(DataIn[1]);
			SerialHexBytePrint(DataIn[0]);
			Serial.println(F(";"));

			I2CBuss.read(SensorAddress, (byte)MLX90614_TA_RAM_REG, (byte)2);

				DataIn[0] = I2CBuss.receive();
				DataIn[1] = I2CBuss.receive();

			Serial.print(F("#MLX90614 TA"));
					SerialHexBytePrint(DataIn[1]);
					SerialHexBytePrint(DataIn[0]);
					Serial.println(F(";"));
#endif /*MLX90614_SEND_OTHER_DATA_IN_COMMENT*/

} //GetTO1DataAndSend()

#ifdef VERIFY_CHIPS
bool SW_MLX90614_Sensor::VerifyChip()
{
	I2CBuss.read(SensorAddress, (byte)MLX90614_CONFIG_EEPROM_REG, (byte)2);

	Serial.print(F("#MLX90614 Config Register "));
	byte RegValue = I2CBuss.receive();
	SerialHexBytePrint(RegValue);
	RegValue = I2CBuss.receive();
	SerialHexBytePrint(RegValue);
	Serial.println(F(";"));

	I2CBuss.read(SensorAddress, (byte)MLX90614_CONFIG_EEPROM_REG, (byte)2);
	RegValue = I2CBuss.receive();

	if(RegValue != (byte)MLX90614_CONFIG_EEPROM_MSB_VALUE)
	{
		Serial.println(F("#MLX90614 Config Register INCORRECT VALUE;"));
		return false;
	}

	RegValue = I2CBuss.receive();
	if(RegValue != (byte)MLX90614_CONFIG_EEPROM_LSB_VALUE)
	{
		Serial.println(F("#MLX90614 Config Register INCORRECT VALUE;"));
		return false;
	}
	else
	{
		Serial.println(F("#MLX90614 Config Register Correct Value;"));
	}

	I2CBuss.read(SensorAddress, (byte)MLX90614_ID_EEPROM_REG, (byte)2);
	RegValue = I2CBuss.receive();

	if(RegValue != (byte)MLX90614_ID_EEPROM_MSB_VALUE)
	{
		Serial.println(F("#MLX90614 INCORRECT ID;"));
		Serial.print(F("#MLX90614 ID "));
		SerialHexBytePrint(RegValue);
		RegValue = I2CBuss.receive();
		SerialHexBytePrint(RegValue);
		Serial.println(F(";"));
		return false;
	}
	else
	{
		Serial.println(F("#MLX90614 Correct ID;"));
	}




	return true;
}
#endif /*VERIFY_CHIPS*/
