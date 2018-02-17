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


#ifdef VERIFY_CHIPS
bool SW_SI1133_Sensor::VerifyChip()
{
	Serial.println(F("#SW_SI1133_Sensor VerifyChip;"));

	byte status = I2CBuss.read(SensorAddress, (byte)SI1133_CHIPID_REG, (byte)1);

	int DataIn = (int)I2CBuss.receive();

	Serial.print(F("#SI1133 Chip ID "));
		SerialHexBytePrint((byte)DataIn);
		Serial.println(F(";"));

		Serial.print(F("#SI1133 Chip ID Read Status "));
				SerialHexBytePrint(status);
				Serial.println(F(";"));

	/*if(DataIn == (int)SI1133_CHIPID_VALUE)
	{
		Serial.println(F("#SI1133 Correct Chip ID;"));
	}
	else
	{
		Serial.println(F("#NOT SI1133 Correct Chip ID;"));
		return false;
	}*/

	Serial.print(F("#SI1133 Rev ID "));
	I2CBuss.read(SensorAddress, (byte)SI1133_REVID_REG, (byte)1);

	DataIn = (int)I2CBuss.receive();
	SerialHexBytePrint((byte)DataIn);
	Serial.println(F(";"));

	Serial.print(F("#SI1133 Mfr ID "));
	I2CBuss.read(SensorAddress, (byte)SI1133_MFRID_REG, (byte)1);

	DataIn = (int)I2CBuss.receive();
	SerialHexBytePrint((byte)DataIn);
	Serial.println(F(";"));


	return true;
}
#endif /*VERIFY_CHIPS*/
