/*
 * SW_SI1133_Sensor.cpp
 *
 *  Created on: 2018-02-13
 *      Author: StoicWeather
 */

#include "SW_SI1133_Sensor.h"

SW_SI1133_Sensor::SW_SI1133_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{


}

bool SW_SI1133_Sensor::InitializeSensor()
{

	Serial.print(F("#Initializing SI1133 Sensor "));
	Serial.print((int)SensorNumber);
	Serial.println(F(" ;"));

	//Send reset
	I2CBuss.write(SensorAddress, (byte)SI1133_COMMAND_REG, (byte)SI1133_RESET_SW_CMD);
	I2CBuss.read(SensorAddress, (byte)SI1133_RESPONSE_0_REG, (byte)1);
	byte DataIn = (int)I2CBuss.receive();

	DataIn = DataIn & SI1133_RESPONSE0_ON_RESET_MASK;

	if(DataIn != (byte)SI1133_RESPONSE0_ON_RESET_VALUE)
	{
		Serial.print(F("#SI1133 Reset FAILED;"));
		return false;
	}



#ifdef VERIFY_CHIPS
	if(!VerifyChip())
	{
		return false;
	}
#endif /*VERIFY_CHIPS*/



	return true;

}

/* Check RESPONSE0 for a CMD_ERR flag.
 * Issues a reset and returns false if error is found unless the
 * error is a saturation or overflow of ADC. For Saturation or overflow returns true and resets.
 * Returns true on no error.
 *
 */
bool SW_SI1133_Sensor::CheckForCMDError()
{
	I2CBuss.read(SensorAddress, (byte)SI1133_RESPONSE_0_REG, (byte)1);
	byte DataIn = (int)I2CBuss.receive();


	if((DataIn & SI1133_RESPONSE0_CMD_ERR_MASK) == (byte)SI1133_RESPONSE0_ON_CMD_ERR_VALUE)
	{
		Serial.print(F("#SI1133 CMD ERROR;"));

		DataIn = DataIn & SI1133_RESPONSE0_CMD_ERR_CODE_MASK;

		if(DataIn == SI1133_CMD_ERR_CODE_SAT_OVERFL_VALUE)
		{
			Serial.print(F("#SI1133 Saturation or Overflow;"));
			I2CBuss.write(SensorAddress, (byte)SI1133_COMMAND_REG, (byte)SI1133_RESET_CMD_CTR_CMD);

			return true;
		}
		else
		{
			I2CBuss.write(SensorAddress, (byte)SI1133_COMMAND_REG, (byte)SI1133_RESET_CMD_CTR_CMD);

			return false;
		}
	}

	return true;

}

/*
 * Checks RESPONSE_0 for a saturation flag and returns false if saturated.
 * Does not perform comand center reset
 */
bool SW_SI1133_Sensor::CheckForSaturation()
{
	I2CBuss.read(SensorAddress, (byte)SI1133_RESPONSE_0_REG, (byte)1);
	byte DataIn = (int)I2CBuss.receive();

	if((DataIn & SI1133_RESPONSE0_CMD_ERR_MASK) == (byte)SI1133_RESPONSE0_ON_CMD_ERR_VALUE)
	{

		DataIn = DataIn & SI1133_RESPONSE0_CMD_ERR_CODE_MASK;

		if(DataIn == SI1133_CMD_ERR_CODE_SAT_OVERFL_VALUE)
		{
			Serial.print(F("#SI1133 Saturation or Overflow;"));

			return false;
		}

	}
	return true;
}


#ifdef VERIFY_CHIPS
bool SW_SI1133_Sensor::VerifyChip()
{
	Serial.println(F("#SW_SI1133_Sensor VerifyChip;"));

	byte status = I2CBuss.read(SensorAddress, (byte)SI1133_PARTID_REG, (byte)1);

	int DataIn = (int)I2CBuss.receive();

	Serial.println(F("SI1133_PARTID_REG"));
	Serial.println(DataIn,HEX);

	status = I2CBuss.read(SensorAddress, (byte)SI1133_REVID_REG, (byte)1);

	DataIn = (int)I2CBuss.receive();

	Serial.println(F("SI1133_REVID_REG"));
	Serial.println(DataIn,HEX);

	status = I2CBuss.read(SensorAddress, (byte)SI1133_MFRID_REG, (byte)1);

	DataIn = (int)I2CBuss.receive();

	Serial.println(F("SI1133_MFRID_REG"));
	Serial.println(DataIn,HEX);

	status = I2CBuss.read(SensorAddress, (byte)SI1133_INFO0_REG, (byte)1);

	DataIn = (int)I2CBuss.receive();

	Serial.println(F("SI1133_INFO0_REG"));
	Serial.println(DataIn,HEX);

	status = I2CBuss.read(SensorAddress, (byte)SI1133_INFO1_REG, (byte)1);

	DataIn = (int)I2CBuss.receive();

	Serial.println(F("SI1133_INFO1_REG"));
	Serial.println(DataIn,HEX);

	return true;

}
#endif /*VERIFY_CHIPS*/
