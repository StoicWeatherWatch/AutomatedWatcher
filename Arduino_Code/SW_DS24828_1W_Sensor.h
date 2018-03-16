/*
 * SW_DS24828_1W_Sensor.h
 *
 *  Created on: 2018-02-21
 *      Author: StoicWeather
 *
 *      The DS2482-800 is a 1 Wire master. Currently it is configured to talk only with DS18B20 temp sensor.
 *
 *      Rather than accessing registers, the DS2482-800 sends commands as a registry pointer. So
 *      reset, DRST, is sent by pointing in write mode at the DRST address and then not writing anything.
 *      A verification can then be read back. DRST is a registr even if nothing is written to it
 *
 *      Lots of commands are sent as simply a blank write to a reg. These are still labeled as reg rather than cmd because
 *      of where they go in the I2C sequence
 *
 *      This is mostly designed for a single 1 Wire dedvice on each channel.
 */

#ifndef SW_DS24828_1W_SENSOR_H_
#define SW_DS24828_1W_SENSOR_H_

#include "SWSensor.h"

#include "SW_Helper_Functions.h"

#define DS24828_DIAGNOSTICS_ON

#define DS24828_CH_0_ACTIVE
#define DS24828_CH_1_ACTIVE
#define DS24828_CH_2_ACTIVE
#define DS24828_CH_3_ACTIVE
#define DS24828_CH_4_ACTIVE
#define DS24828_CH_5_ACTIVE
#define DS24828_CH_6_ACTIVE
#define DS24828_CH_7_ACTIVE

#define DS24828_NUM_CH_ACTIVE 8

#define DS24828_MAX_RETRY_1WCMDS  10

#define DS24828_DRST_REG      0xF0
#define DS24828_WCFG_REG      0xD2
// Select the channel for 1 Wire
#define DS24828_CHSL_REG      0xC3
// 1 wire reset
#define DS24828_1WRS_REG      0xB4

// Set read pointer
#define DS24828_SRP_REG       0xE1


//  /1WS /SPU 1 /APU 1WS SPU 0 APU
//  0b 1111 0000
#define DS24828_WCFG_CMD     0xF0
// 1 wire write byte
#define DS24828_1WWB_CMD     0xA5
// 1 wire read byte
#define DS24828_1WRB_CMD     0x96

// SRP pointers
#define DS24828_SRP_READ_DAT_CMD 0xE1

// 1 Wire commands
#define DS24828_READ_ROM_1WCMD   0x33
#define DS18B20_CONVERT_TEMP_1WCMD   0x44
#define DS18B20_READ_SCRATCH_1WCMD   0xBE
#define DS18B20_SKIP_ROM_1WCMD   0xCC

#define DS24828_CHSL_0_CMD   0xF0
#define DS24828_CHSL_1_CMD   0xE1
#define DS24828_CHSL_2_CMD   0xD2
#define DS24828_CHSL_3_CMD   0xC3
#define DS24828_CHSL_4_CMD   0xB4
#define DS24828_CHSL_5_CMD   0xA5
#define DS24828_CHSL_6_CMD   0x96
#define DS24828_CHSL_7_CMD   0x87


#define DS24828_CHSL_0_VARIFICATION_VALUE   0xB8
#define DS24828_CHSL_1_VARIFICATION_VALUE   0xB1
#define DS24828_CHSL_2_VARIFICATION_VALUE   0xAA
#define DS24828_CHSL_3_VARIFICATION_VALUE   0xA3
#define DS24828_CHSL_4_VARIFICATION_VALUE   0x9C
#define DS24828_CHSL_5_VARIFICATION_VALUE   0x95
#define DS24828_CHSL_6_VARIFICATION_VALUE   0x8E
#define DS24828_CHSL_7_VARIFICATION_VALUE   0x87


#define DS24828_DRST_VARIFICATION_VALUE        0x10
#define DS24828_DRST_VARIFICATION_MASK_VALUE   0xF7
#define DS24828_WCFG_ONRESET_VALUE             0x00





/*
 *
 */
class SW_DS24828_1W_Sensor: public SW_Sensor
{
public:
	byte ActiveChannel;
	SW_DS24828_1W_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);

	bool InitializeSensor();
	bool ResetAll1WDevices();

	bool SelectChannel(int Channel);
	bool SelectNextChannel();

	bool Cmd1W_ResetCurrentCh();

	byte* ReadBlockFrom_1W(byte* Block, byte NumberOfBytes);

	byte WaitFor1WSDone();

	//bool TellDS18B20ToGetTemp_1W(int Channel);
	void Cmd1W_TellDS18B20OnCurrentCHToGetTemp_1W(); // This process takes 750 ms. Do not read temp before
	//int ReadRawTempDA18B20_1W(int Channel);
	//void ReadAndSendRawTempDA18B20_1W(int Channel);
	int ReadRawTempOnCurrentCHDS18B20_1W();
	void ReadAndSendRawTempDA18B20OnCurrentCH_1W();


#ifdef VERIFY_CHIPS
	bool VerifyChip();
	//void ReadROM();
	void Cmd1W_ReadROMCurrentCh();
	void Cmd1W_ReadScratchCurrentCh();
	void Cmd1W_ResetAndReadScratchCurrentCh();
#endif /*VERIFY_CHIPS*/
};

#endif /* SW_DS24828_1W_SENSOR_H_ */
