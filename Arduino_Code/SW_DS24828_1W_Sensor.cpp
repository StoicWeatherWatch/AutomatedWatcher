/*
 * SW_DS24828_1W_Sensor.cpp
 *
 *  Created on: 2018-02-20
 *      Author: StoicWeather
 */

#include "SW_DS24828_1W_Sensor.h"

SW_DS24828_1W_Sensor::SW_DS24828_1W_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{
}

bool SW_DS24828_1W_Sensor::InitializeSensor()
{

	Serial.println(F("#DS24828 InitializeSensor ;"));

	// Device Reset
	byte status = I2CBuss.write(SensorAddress, (byte)DS24828_DRST_REG);

	Serial.print(F("#DS24828 status "));
	SerialHexBytePrint(status);
	Serial.println(F(";"));

#ifdef VERIFY_CHIPS
	status = I2CBuss.read(SensorAddress, (byte)1);
	Serial.print(F("#DS24828 read status "));
	SerialHexBytePrint(status);
	Serial.println(F(";"));

	status = I2CBuss.available();
	Serial.print(F("#DS24828 bytes available "));
	SerialHexBytePrint(status);
	Serial.println(F(";"));

	byte ResetValue = I2CBuss.receive();
	Serial.print(F("#DS24828 ResetValue "));
	SerialHexBytePrint(ResetValue);
	Serial.println(F(";"));
	if((ResetValue & DS24828_DRST_VARIFICATION_MASK_VALUE) == DS24828_DRST_VARIFICATION_VALUE)
	{
		Serial.println(F("#DS24828 Reset Successful;"));
	}
	else
	{
		Serial.println(F("#DS24828 Reset Unsuccessful;"));
	}

#endif /*VERIFY_CHIPS*/

#ifdef VERIFY_CHIPS
	VerifyChip();
#endif /*VERIFY_CHIPS*/

	I2CBuss.write(SensorAddress, (byte)DS24828_WCFG_REG, (byte)DS24828_WCFG_CMD);
	I2CBuss.read(SensorAddress, (byte)DS24828_WCFG_REG, (byte)1);
	byte WCFGValue = I2CBuss.receive();
	Serial.print(F("#DS24828 WCFGValue "));
	SerialHexBytePrint(WCFGValue);
	Serial.println(F(";"));
	if((WCFGValue & 0x0F) == (DS24828_WCFG_CMD & 0x0F))
	{
		Serial.println(F("#DS24828 WCFG Successful;"));
	}
	else
	{
		Serial.println(F("#DS24828 WCFG Unsuccessful;"));
		return false;

	}

	Serial.println(F("#DS24828 calling ResetAll1WDevices;"));
	ResetAll1WDevices();
	if(!ResetAll1WDevices())
	{
		return false;
	}

	return true;
}

bool SW_DS24828_1W_Sensor::ResetAll1WDevices()
{
	Serial.println(F("#DS24828 ResetAll1WDevices;"));

	byte OneWireStatus = 0;


#ifdef DS24828_CH_0_ACTIVE
	SelectChannel(0);

	// Send reset
	I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);
	OneWireStatus = WaitFor1WSDone();

#ifdef VERIFY_CHIPS
	// PPS Presence pulse detect
	if(((OneWireStatus & 0x02) == 0x02) && ((OneWireStatus & 0x04) != 0x04))
	{
		Serial.println(F("#DS24828 ResetAll1WDevices PPS Found 1W Device on Channel 0;"));
	}
	else
	{
		Serial.println(F("#DS24828 ResetAll1WDevices NO PPS Found 1W Device on Channel 0;"));
		return false;
	}

#endif /*VERIFY_CHIPS*/

#endif /*DS24828_CH_0_ACTIVE*/
#ifdef DS24828_CH_1_ACTIVE
	SelectChannel(1);

	// Send reset
	I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);



	OneWireStatus = WaitFor1WSDone();

#ifdef VERIFY_CHIPS
	// PPS Presence pulse detect
	if(((OneWireStatus & 0x02) == 0x02) && ((OneWireStatus & 0x04) != 0x04))
	{
		Serial.println(F("#DS24828 ResetAll1WDevices PPS Found 1W Device on Channel 1;"));
	}
	else
	{
		Serial.println(F("#DS24828 ResetAll1WDevices NO PPS Found 1W Device on Channel 1;"));
		return false;
	}

#endif /*VERIFY_CHIPS*/

#endif /*DS24828_CH_1_ACTIVE*/
#ifdef DS24828_CH_2_ACTIVE
	SelectChannel(2);

	// Send reset
	I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);



	OneWireStatus = WaitFor1WSDone();

#ifdef VERIFY_CHIPS
	// PPS Presence pulse detect
	if((OneWireStatus & 0x02) == 0x02)
	{
		Serial.println(F("#DS24828 ResetAll1WDevices PPS Found 1W Device on Channel 2;"));
	}
	else
	{
		Serial.println(F("#DS24828 ResetAll1WDevices NO PPS Found 1W Device on Channel 2;"));
		return false;
	}

#endif /*VERIFY_CHIPS*/

#endif /*DS24828_CH_2_ACTIVE*/
#ifdef DS24828_CH_3_ACTIVE
	SelectChannel(3);

	// Send reset
	I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);



	OneWireStatus = WaitFor1WSDone();

#ifdef VERIFY_CHIPS
	// PPS Presence pulse detect
	if(((OneWireStatus & 0x02) == 0x02) && ((OneWireStatus & 0x04) != 0x04))
	{
		Serial.println(F("#DS24828 ResetAll1WDevices PPS Found 1W Device on Channel 3;"));
	}
	else
	{
		Serial.println(F("#DS24828 ResetAll1WDevices NO PPS Found 1W Device on Channel 3;"));
		return false;
	}

#endif /*VERIFY_CHIPS*/

#endif /*DS24828_CH_3_ACTIVE*/
#ifdef DS24828_CH_4_ACTIVE
	SelectChannel(4);

	// Send reset
	I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);



	OneWireStatus = WaitFor1WSDone();

#ifdef VERIFY_CHIPS
	// PPS Presence pulse detect
	if((OneWireStatus & 0x02) == 0x02)
	{
		Serial.println(F("#DS24828 ResetAll1WDevices PPS Found 1W Device on Channel 4;"));
	}
	else
	{
		Serial.println(F("#DS24828 ResetAll1WDevices NO PPS Found 1W Device on Channel 4;"));
		return false;
	}

#endif /*VERIFY_CHIPS*/

#endif /*DS24828_CH_4_ACTIVE*/
#ifdef DS24828_CH_5_ACTIVE
	SelectChannel(5);

	// Send reset
	I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);



	OneWireStatus = WaitFor1WSDone();

#ifdef VERIFY_CHIPS
	// PPS Presence pulse detect
	if((OneWireStatus & 0x02) == 0x02)
	{
		Serial.println(F("#DS24828 ResetAll1WDevices PPS Found 1W Device on Channel 5;"));
	}
	else
	{
		Serial.println(F("#DS24828 ResetAll1WDevices NO PPS Found 1W Device on Channel 5;"));
		return false;
	}

#endif /*VERIFY_CHIPS*/

#endif /*DS24828_CH_5_ACTIVE*/
#ifdef DS24828_CH_6_ACTIVE
	SelectChannel(6);

	// Send reset
	I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);



	OneWireStatus = WaitFor1WSDone();

#ifdef VERIFY_CHIPS
	// PPS Presence pulse detect
	if((OneWireStatus & 0x02) == 0x02)
	{
		Serial.println(F("#DS24828 ResetAll1WDevices PPS Found 1W Device on Channel 6;"));
	}
	else
	{
		Serial.println(F("#DS24828 ResetAll1WDevices NO PPS Found 1W Device on Channel 6;"));
		return false;
	}

#endif /*VERIFY_CHIPS*/

#endif /*DS24828_CH_6_ACTIVE*/
#ifdef DS24828_CH_7_ACTIVE
	SelectChannel(7);

	// Send reset
	I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);



	OneWireStatus = WaitFor1WSDone();

#ifdef VERIFY_CHIPS
	// PPS Presence pulse detect
	if((OneWireStatus & 0x02) == 0x02)
	{
		Serial.println(F("#DS24828 ResetAll1WDevices PPS Found 1W Device on Channel 7;"));
	}
	else
	{
		Serial.println(F("#DS24828 ResetAll1WDevices NO PPS Found 1W Device on Channel 7;"));
		return false;
	}

#endif /*VERIFY_CHIPS*/

#endif /*DS24828_CH_7_ACTIVE*/


	return true;
}

bool SW_DS24828_1W_Sensor::SelectChannel(int Channel)
{
	byte ChCMD = 0;
	byte ChVari = 0;
	switch (Channel)
	{
#ifdef DS24828_CH_0_ACTIVE
	case 0:
		ChCMD = DS24828_CHSL_0_CMD;
		ChVari = DS24828_CHSL_0_VARIFICATION_VALUE;
		break;
#endif /*DS24828_CH_0_ACTIVE*/
#ifdef DS24828_CH_1_ACTIVE
	case 1:
		ChCMD = DS24828_CHSL_1_CMD;
		ChVari = DS24828_CHSL_1_VARIFICATION_VALUE;
		break;
#endif /*DS24828_CH_1_ACTIVE*/
#ifdef DS24828_CH_2_ACTIVE
	case 2:
		ChCMD = DS24828_CHSL_2_CMD;
		ChVari = DS24828_CHSL_2_VARIFICATION_VALUE;
		break;
#endif /*DS24828_CH_2_ACTIVE*/
#ifdef DS24828_CH_3_ACTIVE
	case 3:
		ChCMD = DS24828_CHSL_3_CMD;
		ChVari = DS24828_CHSL_3_VARIFICATION_VALUE;
		break;
#endif /*DS24828_CH_3_ACTIVE*/
#ifdef DS24828_CH_4_ACTIVE
	case :
		ChCMD = DS24828_CHSL_4_CMD;
		ChVari = DS24828_CHSL_4_VARIFICATION_VALUE;
		break;
#endif /*DS24828_CH_4_ACTIVE*/
#ifdef DS24828_CH_5_ACTIVE
	case :
		ChCMD = DS24828_CHSL_5_CMD;
		ChVari = DS24828_CHSL_5_VARIFICATION_VALUE;
		break;
#endif /*DS24828_CH_5_ACTIVE*/
#ifdef DS24828_CH_6_ACTIVE
	case :
		ChCMD = DS24828_CHSL_6_CMD;
		ChVari = DS24828_CHSL_6_VARIFICATION_VALUE;
		break;
#endif /*DS24828_CH_6_ACTIVE*/
#ifdef DS24828_CH_7_ACTIVE
	case :
		ChCMD = DS24828_CHSL_7_CMD;
		ChVari = DS24828_CHSL_7_VARIFICATION_VALUE;
		break;
#endif /*DS24828_CH_7_ACTIVE*/

	default:
		Serial.print(F("#DS24828 SelectChannel Failed. Channel not active "));
		Serial.print(Channel);
		Serial.println(F(";"));
		return false;
	}

	I2CBuss.write(SensorAddress, (byte)DS24828_CHSL_REG, ChCMD);
	I2CBuss.read(SensorAddress, (byte)1);
	byte CHSLValue = I2CBuss.receive();
	Serial.print(F("#DS24828 CHSLValue "));
	SerialHexBytePrint(CHSLValue);
	Serial.println(F(";"));
	if(CHSLValue == ChVari)
	{
		Serial.println(F("#DS24828 CHSL Successful;"));
	}
	else
	{
		Serial.println(F("#DS24828 CHSL Unsuccessful;"));
		return false;
	}



	return true;

}

bool SW_DS24828_1W_Sensor::TellDS18B20ToGetTemp_1W(int Channel)
{
	if(!SelectChannel(Channel))
	{
		return false;
	}

	// Reset the 1W Device
	I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);
	byte OneWireStatus = WaitFor1WSDone();

	// Give the command to skip ROM. Only one device per channel.
	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS18B20_SKIP_ROM_1WCMD);
	 OneWireStatus = WaitFor1WSDone();

	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS18B20_CONVERT_TEMP_1WCMD);
	 OneWireStatus = WaitFor1WSDone();

	return true;

}
	int SW_DS24828_1W_Sensor::ReadRawTempDA18B20_1W(int Channel)
	{
		if(!SelectChannel(Channel))
			{
				return 0;
			}

		// Give the command to skip ROM. Only one device per channel.
			I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS18B20_SKIP_ROM_1WCMD);
			byte OneWireStatus = WaitFor1WSDone();

		// Tell the DS18B20 to send data
		I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS18B20_READ_SCRATCH_1WCMD);

		 OneWireStatus = WaitFor1WSDone();
		if((OneWireStatus & 0x01) != 0x00)
		{
			return 0;
		}

		// Tell DS2482-800 to read data
		I2CBuss.write(SensorAddress, (byte)DS24828_1WRB_CMD);
		 OneWireStatus = WaitFor1WSDone();
				if((OneWireStatus & 0x01) != 0x00)
				{
					return 0;
				}

		I2CBuss.write(SensorAddress, (byte)DS24828_SRP_REG, (byte)DS24828_SRP_READ_DAT_CMD);
		I2CBuss.read(SensorAddress, (byte)1);
		byte VeryRawData0 = I2CBuss.receive();

		// Repeat for the second byte
		I2CBuss.write(SensorAddress, (byte)DS24828_1WRB_CMD);
				 OneWireStatus = WaitFor1WSDone();
						if((OneWireStatus & 0x01) != 0x00)
						{
							return 0;
						}

						I2CBuss.write(SensorAddress, (byte)DS24828_SRP_REG, (byte)DS24828_SRP_READ_DAT_CMD);
								I2CBuss.read(SensorAddress, (byte)1);
								byte VeryRawData1 = I2CBuss.receive();

								// Reset the 1W Device
									I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);
									 OneWireStatus = WaitFor1WSDone();

								return ( (int)VeryRawData0 + (((int)VeryRawData1) << 8) );



	}
	void SW_DS24828_1W_Sensor::ReadAndSendRawTempDA18B20_1W(int Channel)
	{
		int VERYRawData = ReadRawTempDA18B20_1W(Channel);

		Serial.print(F("*"));
		Serial.print(SensorNumber+Channel,DEC);
		Serial.print(F("T,"));
		SerialHexBytePrint((byte)(VERYRawData >> 8));
		SerialHexBytePrint((byte)(VERYRawData));
		Serial.println(F(";"));

	}

#ifdef VERIFY_CHIPS
// VerifyChip assumes a DRST has just happened
bool SW_DS24828_1W_Sensor::VerifyChip()
{
	I2CBuss.read(SensorAddress, (byte)DS24828_WCFG_REG, (byte)1);
	byte WCFGValue = I2CBuss.receive();
	if(WCFGValue == DS24828_WCFG_ONRESET_VALUE)
	{
		Serial.println(F("#DS24828 WCFG Verified;"));
	}
	else
	{
		Serial.println(F("#DS24828 WCFG NOT Verified;"));
		return false;
	}

	// Read the ROM codes

	byte OneWireStatus = 0;
	byte ROMValue = 0;

#ifdef DS24828_CH_0_ACTIVE
	SW_DS24828_1W_Sensor::SelectChannel(0);
	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS24828_READ_ROM_1WCMD);
	OneWireStatus = WaitFor1WSDone();

	Serial.print(F("#DS24828 CH 0 ROM  "));
	ReadROM();
	Serial.println(F(";"));

#endif /*DS24828_CH_0_ACTIVE*/
#ifdef DS24828_CH_1_ACTIVE
	SW_DS24828_1W_Sensor::SelectChannel(1);
	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS24828_READ_ROM_1WCMD);


	OneWireStatus = WaitFor1WSDone();
	Serial.print(F("#DS24828 CH 1 ROM  "));
	ReadROM();
	Serial.println(F(";"));

#endif /*DS24828_CH_1_ACTIVE*/
#ifdef DS24828_CH_2_ACTIVE
	SW_DS24828_1W_Sensor::SelectChannel(2);
	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS24828_READ_ROM_1WCMD);


	OneWireStatus = WaitFor1WSDone();
	// Tell the DS2482 to send back a byte
	I2CBuss.write(SensorAddress, (byte)DS24828_SRP_REG, (byte)DS24828_SRP_READ_DAT_CMD);

	I2CBuss.read(SensorAddress, (byte)1);
	 ROMValue = I2CBuss.receive();
	Serial.print(F("#DS24828 CH 2 ROM  "));
	SerialHexBytePrint(ROMValue);
	Serial.println(F(";"));

#endif /*DS24828_CH_2_ACTIVE*/
#ifdef DS24828_CH_3_ACTIVE
	SW_DS24828_1W_Sensor::SelectChannel(3);
	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS24828_READ_ROM_1WCMD);


	OneWireStatus = WaitFor1WSDone();

	Serial.print(F("#DS24828 CH 3 ROM  "));

		ReadROM();
		Serial.println(F(";"));


#endif /*DS24828_CH_3_ACTIVE*/


	return true;


} // VerifyChip()

void SW_DS24828_1W_Sensor::ReadROM()
{
	// ROM code is 64 bits, 8 bytes
	byte OneWireStatus;
	byte ROMValue = 0x22;
		for(int i = 0; i <8;i++)
		{

		// Tell the DS2482 to send back a byte
		I2CBuss.write(SensorAddress, (byte)DS24828_1WRB_CMD);
		OneWireStatus = WaitFor1WSDone();
		I2CBuss.write(SensorAddress, (byte)DS24828_SRP_REG, (byte)DS24828_SRP_READ_DAT_CMD);
		I2CBuss.read(SensorAddress, (byte)1);
		 ROMValue = I2CBuss.receive();
		SerialHexBytePrint(ROMValue);
		}


}

#endif /*VERIFY_CHIPS*/


// TODO Disable if never called.
byte SW_DS24828_1W_Sensor::WaitFor1WSDone()
{
	byte OneWireStatus;
	byte readStatus;

		for(int i = 0; i < DS24828_MAX_RETRY_1WCMDS; i++)
			{

			    readStatus = I2CBuss.read(SensorAddress, (byte)1);
				OneWireStatus = I2CBuss.receive();
				/*Serial.print(F("#DS24828 WaitFor1WSDone OneWireStatus "));
				Serial.print(OneWireStatus);
				Serial.println(F(";"));*/

				//Status Register Bit Assignment bit 7 bit 6 bit 5 bit 4 bit 3 bit 2 bit 1 bit 0
				//DIR TSB SBR RST LL SD PPD 1WB
				if(((OneWireStatus & 0x01) == 0x00) && (readStatus == 0))
				{
					return OneWireStatus;
				}
			}

		Serial.println(F("#DS24828 WaitFor1WSDone time out or other fail;"));
		return 0x01;
}
