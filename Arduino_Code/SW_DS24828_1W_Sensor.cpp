/*
 * SW_DS24828_1W_Sensor.cpp
 *
 *  Created on: 2018-02-21
 *      Author: StoicWeather
 */

#include "SW_DS24828_1W_Sensor.h"

SW_DS24828_1W_Sensor::SW_DS24828_1W_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{
	ActiveChannel = 0;
}

bool SW_DS24828_1W_Sensor::InitializeSensor()
{

	Serial.println(F("#DS24828 InitializeSensor ;"));

	// Device Reset
	I2CBuss.write(SensorAddress, (byte)DS24828_DRST_REG);

	// TODO Remove status and clean up


#ifdef VERIFY_CHIPS
	byte status;
	status = I2CBuss.read(SensorAddress, (byte)1);

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

	SelectChannel(ActiveChannel);

	return true;
}

bool SW_DS24828_1W_Sensor::ResetAll1WDevices()
{
	Serial.println(F("#DS24828 ResetAll1WDevices;"));

	byte OneWireStatus = 0;


#ifdef DS24828_CH_0_ACTIVE
	SelectChannel(0);

	Cmd1W_ResetCurrentCh();

#endif /*DS24828_CH_0_ACTIVE*/
#ifdef DS24828_CH_1_ACTIVE
	SelectChannel(1);

	Cmd1W_ResetCurrentCh();

#endif /*DS24828_CH_1_ACTIVE*/
#ifdef DS24828_CH_2_ACTIVE
	SelectChannel(2);

	Cmd1W_ResetCurrentCh();

#endif /*DS24828_CH_2_ACTIVE*/
#ifdef DS24828_CH_3_ACTIVE
	SelectChannel(3);

	Cmd1W_ResetCurrentCh();

#endif /*DS24828_CH_3_ACTIVE*/
#ifdef DS24828_CH_4_ACTIVE
	SelectChannel(4);

	Cmd1W_ResetCurrentCh();

#endif /*DS24828_CH_4_ACTIVE*/
#ifdef DS24828_CH_5_ACTIVE
	SelectChannel(5);

	Cmd1W_ResetCurrentCh();

#endif /*DS24828_CH_5_ACTIVE*/
#ifdef DS24828_CH_6_ACTIVE
	SelectChannel(6);

	Cmd1W_ResetCurrentCh();

#endif /*DS24828_CH_6_ACTIVE*/
#ifdef DS24828_CH_7_ACTIVE
	SelectChannel(7);

	Cmd1W_ResetCurrentCh();

#endif /*DS24828_CH_7_ACTIVE*/

	SelectChannel(ActiveChannel);


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

		ActiveChannel = Channel;
	}
	else
	{
		Serial.println(F("#DS24828 CHSL Unsuccessful;"));
		return false;
	}



	return true;

}

bool SW_DS24828_1W_Sensor::SelectNextChannel()
{
	ActiveChannel++;
	if(ActiveChannel >= DS24828_NUM_CH_ACTIVE)
	{
		ActiveChannel = 0;
	}

#ifndef DS24828_CH_2_ACTIVE
	if(ActiveChannel == 2)
		{
			ActiveChannel = 3;
		}
#endif /*DS24828_CH_2_ACTIVE*/

	return SelectChannel(ActiveChannel);

}

byte* SW_DS24828_1W_Sensor::ReadBlock1W(byte* Block, byte NumberOfBytes)
/*A ROM selection or ignore ROM command must come before this. Or a command to read ROM if only one chip on the channel.*/
{

	byte OneWireStatus; // TODO OneWireStatus not used
	OneWireStatus = WaitFor1WSDone();

	for(int i = 0; i <NumberOfBytes;i++)
	{

		// Tell the DS2482 to send back a byte
		I2CBuss.write(SensorAddress, (byte)DS24828_1WRB_CMD);
		OneWireStatus = WaitFor1WSDone();
		I2CBuss.write(SensorAddress, (byte)DS24828_SRP_REG, (byte)DS24828_SRP_READ_DAT_CMD);
		I2CBuss.read(SensorAddress, (byte)1);
		Block[i] = I2CBuss.receive();
	}

	return Block;
}

bool SW_DS24828_1W_Sensor::Cmd1W_ResetCurrentCh()
{
	I2CBuss.write(SensorAddress, (byte)DS24828_1WRS_REG);
	byte OneWireStatus = WaitFor1WSDone();

	/*Serial.print(F("# DS24828 Cmd1W_ResetCurrentCh OneWireStatus "));
					Serial.print(OneWireStatus);
					Serial.println(F(";"));*/

#ifdef VERIFY_CHIPS
	// PPS Presence pulse detect
	if(((OneWireStatus & 0x02) == 0x02) && ((OneWireStatus & 0x04) != 0x04))
	{
		Serial.println(F("#DS24828 Cmd1W_ResetCurrentCh PPS Found 1W Device on current Channel;"));
	}
	else
	{
		Serial.println(F("#DS24828 Cmd1W_ResetCurrentCh NO PPS Found 1W Device on current Channel - ERROR;"));
		return false;
	}

#endif /*VERIFY_CHIPS*/

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
void SW_DS24828_1W_Sensor::Cmd1W_TellDS18B20OnCurrentCHToGetTemp_1W()
{
#ifdef DS24828_DIAGNOSTICS_ON
	byte OneWireStatus;
#endif /*DS24828_DIAGNOSTICS_ON*/

	// Reset the 1W Device
	Cmd1W_ResetCurrentCh();
	WaitFor1WSDone();

	// Give the command to skip ROM. Only one device per channel.
	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS18B20_SKIP_ROM_1WCMD);
#ifndef DS24828_DIAGNOSTICS_ON
	WaitFor1WSDone();
#endif /*not DS24828_DIAGNOSTICS_ON*/
#ifdef DS24828_DIAGNOSTICS_ON
	OneWireStatus = WaitFor1WSDone();
#endif /*DS24828_DIAGNOSTICS_ON*/

	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS18B20_CONVERT_TEMP_1WCMD);
#ifndef DS24828_DIAGNOSTICS_ON
	WaitFor1WSDone();
#endif /*not DS24828_DIAGNOSTICS_ON*/
#ifdef DS24828_DIAGNOSTICS_ON
	OneWireStatus = WaitFor1WSDone();
#endif /*DS24828_DIAGNOSTICS_ON*/

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



} //ReadRawTempDA18B20_1W

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

int SW_DS24828_1W_Sensor::ReadRawTempOnCurrentCHDS18B20_1W()
{

	Cmd1W_ResetCurrentCh();
	WaitFor1WSDone();

	// Give the command to skip ROM. Only one device per channel.
	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS18B20_SKIP_ROM_1WCMD);
	WaitFor1WSDone();

	// Tell the DS18B20 to send data
	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS18B20_READ_SCRATCH_1WCMD);
	WaitFor1WSDone();

	byte* TempBlock = (byte*) calloc(2,sizeof(byte));

	TempBlock = ReadBlock1W(TempBlock, 2);


	int TempRaw = ( (int)TempBlock[0] + (((int)TempBlock[1]) << 8) );

	free(TempBlock);

	return TempRaw;



}

void SW_DS24828_1W_Sensor::ReadAndSendRawTempDA18B20OnCurrentCH_1W()
{
	int VERYRawData = ReadRawTempOnCurrentCHDS18B20_1W();

	// LSB comes in first then MSB so the order must be reversed.

	Serial.print(F("*"));
	Serial.print(SensorNumber+ActiveChannel,DEC);
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
	SelectChannel(0);

	Cmd1W_ResetCurrentCh();

	Cmd1W_ReadROMCurrentCh();
	Cmd1W_ResetAndReadScratchCurrentCh();


#endif /*DS24828_CH_0_ACTIVE*/
#ifdef DS24828_CH_1_ACTIVE
	SelectChannel(1);
	Cmd1W_ResetCurrentCh();

		Cmd1W_ReadROMCurrentCh();
		Cmd1W_ResetAndReadScratchCurrentCh();

#endif /*DS24828_CH_1_ACTIVE*/
#ifdef DS24828_CH_2_ACTIVE
	SelectChannel(2);
	Cmd1W_ResetCurrentCh();

		Cmd1W_ReadROMCurrentCh();
		Cmd1W_ResetAndReadScratchCurrentCh();

#endif /*DS24828_CH_2_ACTIVE*/
#ifdef DS24828_CH_3_ACTIVE
	SelectChannel(3);
	Cmd1W_ResetCurrentCh();

		Cmd1W_ReadROMCurrentCh();
		Cmd1W_ResetAndReadScratchCurrentCh();


#endif /*DS24828_CH_3_ACTIVE*/
#ifdef DS24828_CH_4_ACTIVE
	SelectChannel(4);
	Cmd1W_ResetCurrentCh();

		Cmd1W_ReadROMCurrentCh();
		Cmd1W_ResetAndReadScratchCurrentCh();


#endif /*DS24828_CH_4_ACTIVE*/
#ifdef DS24828_CH_5_ACTIVE
	SelectChannel(5);
	Cmd1W_ResetCurrentCh();

		Cmd1W_ReadROMCurrentCh();
		Cmd1W_ResetAndReadScratchCurrentCh();


#endif /*DS24828_CH_5_ACTIVE*/
#ifdef DS24828_CH_6_ACTIVE
	SelectChannel(6);
	Cmd1W_ResetCurrentCh();

		Cmd1W_ReadROMCurrentCh();
		Cmd1W_ResetAndReadScratchCurrentCh();


#endif /*DS24828_CH_6_ACTIVE*/
#ifdef DS24828_CH_7_ACTIVE
	SelectChannel(7);
	Cmd1W_ResetCurrentCh();

		Cmd1W_ReadROMCurrentCh();
		Cmd1W_ResetAndReadScratchCurrentCh();


#endif /*DS24828_CH_7_ACTIVE*/


		SelectChannel(0);

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

void SW_DS24828_1W_Sensor::Cmd1W_ReadROMCurrentCh()
{
	byte OneWireStatus;
	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS24828_READ_ROM_1WCMD);
	OneWireStatus = WaitFor1WSDone();

	Serial.print(F("#DS24828 CH "));
	SerialHexBytePrint(ActiveChannel);
	Serial.print(F(" ROM  "));


	byte* ROMBlock = (byte*) calloc(8,sizeof(byte));
	ReadBlock1W(ROMBlock, 8);

	for(int i =0; i<8;i++)
	{
		SerialHexBytePrint(ROMBlock[i]);
	}


	Serial.println(F(";"));
	free(ROMBlock);
}

void SW_DS24828_1W_Sensor::Cmd1W_ResetAndReadScratchCurrentCh()
{

	// 1 wire reset
	Cmd1W_ResetCurrentCh();

	Cmd1W_ReadScratchCurrentCh();
}

void SW_DS24828_1W_Sensor::Cmd1W_ReadScratchCurrentCh()
{
	byte OneWireStatus;

	// Give the command to skip ROM. Only one device per channel.
	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS18B20_SKIP_ROM_1WCMD);
	OneWireStatus = WaitFor1WSDone();


	I2CBuss.write(SensorAddress, (byte)DS24828_1WWB_CMD, (byte)DS18B20_READ_SCRATCH_1WCMD);
	OneWireStatus = WaitFor1WSDone();

	Serial.print(F("#DS24828 CH "));
	SerialHexBytePrint(ActiveChannel);
	Serial.print(F(" Scratch  "));


	byte* ScratchBlock = (byte*) calloc(9,sizeof(byte));
	ReadBlock1W(ScratchBlock, 9);

	for(int i =0; i<9;i++)
	{
		SerialHexBytePrint(ScratchBlock[i]);
	}


	Serial.println(F(";"));
	free(ScratchBlock);
}

#endif /*VERIFY_CHIPS*/



byte SW_DS24828_1W_Sensor::WaitFor1WSDone()
{
	byte OneWireStatus;
	byte readStatus; // TODO No real need for readStatus

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
