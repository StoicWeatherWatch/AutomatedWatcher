/*
 * SW_ChipCap2_Sensor.cpp
 *
 *  Created on: 2018-03-16
 *      Author: StoicWeather
 */

#include "SW_ChipCap2_Sensor.h"

SW_ChipCap2_Sensor::SW_ChipCap2_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN), SensorInitialized(false)
{
#ifdef VERIFY_CHIPS
	RevisionNum = 0;
#endif /*VERIFY_CHIPS*/

}

// Unlike in other sensors, this InitializeSensor happens before serial is setup. Therefore the status is saved for later printing.
bool SW_ChipCap2_Sensor::InitializeSensor()
{

	byte DataHolder[2];
	DataHolder[0] = 0x00;
	DataHolder[1] = 0x00;

	// Send START_CM and wait 100 us
	I2CBuss.write(SensorAddress, (byte)CHIPCAP2_STRT_CM_CMD, DataHolder, (byte)2);
	delay(1);

	I2CBuss.read(SensorAddress, (byte)2, DataHolder);
	if(DataHolder[0] != CHIPCAP2_STRT_CM_RESPONSE_1_VALUE)
	{
		SensorInitialized = false;
		return false;
	}


#ifdef VERIFY_CHIPS
	RevisionNum = DataHolder[1];
#endif /*VERIFY_CHIPS*/


#ifdef VERIFY_CHIPS
	if(!VerifyChip())
	{
		SensorInitialized = false;
		return false;
	}
#endif /*VERIFY_CHIPS*/

	// Send START NOM
	I2CBuss.write(SensorAddress, (byte)CHIPCAP2_STRT_NOM_CMD, DataHolder, (byte)2);
	I2CBuss.read(SensorAddress, (byte)2, DataHolder);

	SensorInitialized = true;
	return true;
} // InitializeSensor


void SW_ChipCap2_Sensor::SendMeasurmentRequest()
{

	I2CBuss.write(SensorAddress, (byte)0x00);


} //SendMeasurmentRequest
	void SW_ChipCap2_Sensor::PerformDataFetch()
	{
		byte DataHolder[4];
			DataHolder[0] = 0x00;
			DataHolder[1] = 0x00;
			DataHolder[2] = 0x00;
			DataHolder[3] = 0x00;

		I2c.read(SensorAddress, (byte)4, DataHolder);

		//Is it valid data?
		if((DataHolder[0] & 0b11000000) == 0x00)
		{
			Serial.print(F("*"));
						Serial.print(SensorNumber,DEC);
						Serial.print(F("TH,"));
						SerialHexBytePrint(DataHolder[0]);
						SerialHexBytePrint(DataHolder[1]);
						SerialHexBytePrint(DataHolder[2]);
						SerialHexBytePrint(DataHolder[3]);
						Serial.println(F(";"));
		}
		else
		{
			Serial.print(F("# Bad Data "));
									Serial.print(SensorNumber,DEC);
									Serial.print(F("TH,"));
									SerialHexBytePrint(DataHolder[0]);
									Serial.println(F(" ;"));
		}



	} //PerformDataFetch

void SW_ChipCap2_Sensor::ReportInitialization()
{
	if(SensorInitialized)
	{
	Serial.print(F("#ChipCap2 Sensor "));
	Serial.print((int)SensorNumber);
	Serial.println(F(" Initialized Successfully;"));
	}
	else
	{
		Serial.println(F("#ChipCap2 Sensor FAILED to Initialize (Hopefully it initialized itself:);"));

	}

#ifdef VERIFY_CHIPS
	Serial.print(F("#ChipCap2 Revision Number "));
		Serial.print(RevisionNum,HEX);
		Serial.println(F(";"));
#endif /*VERIFY_CHIPS*/

}


#ifdef VERIFY_CHIPS
	bool SW_ChipCap2_Sensor::VerifyChip()
	{



		return true;
	}
#endif /*VERIFY_CHIPS*/
