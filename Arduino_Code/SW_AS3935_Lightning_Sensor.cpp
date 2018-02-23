/*
 * SW_AS3935_Lightning_Sensor.cpp
 *
 *  Created on: 2018-02-22
 *      Author: StoicWeather
 */

#include "SW_AS3935_Lightning_Sensor.h"

SW_AS3935_Lightning_Sensor::SW_AS3935_Lightning_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN, byte LightningIRQDigPinIn)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{
	LightningIRQDigPin = LightningIRQDigPinIn;
	IRQReceived = false;

}

bool SW_AS3935_Lightning_Sensor::InitializeSensor()
{

	Serial.print(F("#Initializing AS3935 Sensor "));
	Serial.print((int)SensorNumber);
	Serial.println(F(" ;"));

	pinMode(LightningIRQDigPin, INPUT);

	// Restore Defaults
	I2CBuss.write(SensorAddress, (byte)AS3935_PRESET_DEFAULT_REG, (byte)AS3935_EXE_ACTION_CMD);

	// Set Analog Front-end (AFE) for indoor or outdoor
	I2CBuss.write(SensorAddress, (byte)AS3935_AFE_GB_PWD_REG, (byte)AS3935_AFE_GB_PWD_CMD);

	// Set TUNE CAP
	I2CBuss.write(SensorAddress, (byte)AS3935_TUNECAP_IRQ_CONTROL_REG, (byte)AS3935_TUNECAP_IRQ_CONTROL_CMD);

	// calibrate RCO - Calibrates automatically the internal RC Oscillators
	// 1. Send Direct command CALIB_RCO 2. Modify REG0x08[5] = 1
	//3. Wait 2ms
	//4. Modify REG0x08[5] = 0
	// Data sheet from Embedded Adventures says o Set Register 0x08, bit 5 to 1  Wait 2 milliseconds  Set Register 0x08, bit 5 to 0
	// This spits TRCO out of the IRQ pin. - As it happens this is how to do it
	delay(3);
	I2CBuss.write(SensorAddress, (byte)AS3935_CALIB_RCO_REG, (byte)AS3935_EXE_ACTION_CMD);
	I2CBuss.write(SensorAddress, (byte)AS3935_TUNECAP_IRQ_CONTROL_REG, (byte)AS3935_TUNECAP_IRQ_CONTROL_TRCO_ON_IRQ_CMD);
	delay(3);
	I2CBuss.write(SensorAddress, (byte)AS3935_TUNECAP_IRQ_CONTROL_REG, (byte)AS3935_TUNECAP_IRQ_CONTROL_CMD);

	// Set WDTH Watchdog threshold
	I2CBuss.write(SensorAddress, (byte)AS3935_NF_LEV_WDTH_REG, (byte)AS3935_NF_LEV_WDTH_CMD);

	// Reset stats
	I2CBuss.write(SensorAddress, (byte)AS3935_CLRSTAT_LMIN_SREJ_REG, (byte)AS3935_CLRSTAT_LMIN_SREJ_FINAL_CMD);
	//delay(1);
	I2CBuss.write(SensorAddress, (byte)AS3935_CLRSTAT_LMIN_SREJ_REG, (byte)AS3935_CLRSTAT_LMIN_SREJ_TOGGEL_CMD);
	//delay(1);
	I2CBuss.write(SensorAddress, (byte)AS3935_CLRSTAT_LMIN_SREJ_REG, (byte)AS3935_CLRSTAT_LMIN_SREJ_FINAL_CMD);


#ifdef VERIFY_CHIPS
	VerifyChip();
#endif /*VERIFY_CHIPS*/


	return true;
}

void SW_AS3935_Lightning_Sensor::CheckIRQ()
{
	if(digitalRead(LightningIRQDigPin) == HIGH)
	{
		IRQReceived = true;
	}
}
	void SW_AS3935_Lightning_Sensor::IfIRQGetDataAndSend()
	{
		if(IRQReceived)
		{
			Serial.println(F("#AS3935 Sensor IRQ Received;"));
			IRQReceived = false;

			// CHeck the IRQ status bits
			I2CBuss.read(SensorAddress, (byte)AS3935_INT_REG, (byte)1);
			byte RegValue = I2CBuss.receive();
			RegValue = RegValue & AS3935_INT_MASK;
			Serial.print(F("#AS3935 Sensor status bits "));
			SerialHexBytePrint(RegValue);
			Serial.println(F(";"));

			if(RegValue == AS3935_INT_NH_VALUE)
			{
				Serial.println(F("#AS3935 Sensor Noise level too high;"));
				Serial.println(F("!AS3935Noise;"));
			}
			else if(RegValue == AS3935_INT_D_VALUE)
			{
				Serial.println(F("#AS3935 Sensor Disturber detected;"));
				Serial.println(F("!AS3935Disturber;"));
			}
			else if(RegValue == AS3935_INT_L_VALUE)
						{
							Serial.println(F("#AS3935 Sensor Lightning interrupt;"));
							GetAndSendData();
						}
			else if(RegValue == AS3935_INT_L0_VALUE)
									{
										Serial.println(F("#AS3935 Sensor Lightning interrupt - Distance changed due to purging of old events;"));
										Serial.println(F("!AS3935Purge;"));
									}
			else
			{
				Serial.println(F("#AS3935 Sensor Some other INT value - This should never happen;"));
			}


		}
		else
		{
			Serial.println(F("#AS3935 Sensor No IRQ;"));
		}

	} // IfIRQGetDataAndSend

	void SW_AS3935_Lightning_Sensor::GetAndSendData()
	{
		// Read distance
		I2CBuss.read(SensorAddress, (byte)AS3935_LIGHTNING_DISTANCE_REG, (byte)1);
			byte RegValue = I2CBuss.receive();
					RegValue = RegValue & AS3935_LIGHTNING_DISTANCE_MASK;
		Serial.print(F("*"));
			Serial.print(SensorNumber,DEC);
			Serial.print(F("EM,"));
			SerialHexBytePrint(RegValue);
			Serial.println(F(";"));


#ifdef SEND_MEANINGLESS_ENERGY
			// Read meaningless energy
			I2CBuss.read(SensorAddress, (byte)AS3935_LIGHTNING_DISTANCE_REG, (byte)3);
						 RegValue = I2CBuss.receive();
					Serial.print(F("#"));
						Serial.print(SensorNumber,DEC);
						Serial.print(F("EM_MEANINGLESS_ENERGY,"));
						SerialHexBytePrint(RegValue);
						 RegValue = I2CBuss.receive();
						SerialHexBytePrint(RegValue);
						RegValue = I2CBuss.receive();
						RegValue = RegValue & AS3935_LIGHTNING_ENERGY_MMSB_MASK;
						SerialHexBytePrint(RegValue);
						Serial.println(F(";"));
#endif /*SEND_MEANINGLESS_ENERGY*/

	}


#ifdef VERIFY_CHIPS
bool SW_AS3935_Lightning_Sensor::VerifyChip()
{
	Serial.println(F("#AS3935 Sensor VerifyChip;"));

	// Confirm tune cap
	I2CBuss.read(SensorAddress, (byte)AS3935_TUNECAP_IRQ_CONTROL_REG, (byte)1);
	byte RegValue = I2CBuss.receive();

	if(RegValue == (byte)AS3935_TUNECAP_IRQ_CONTROL_CMD)
	{
		Serial.println(F("#AS3935 Sensor Lightning TUNE_CAP Verified;"));
	}
	else
	{
		Serial.println(F("#AS3935 Sensor Lightning TUNE_CAP is DIFFERENT - ERROR;"));
		return false;
	}

	// Confirm clr stat
	I2CBuss.read(SensorAddress, (byte)AS3935_CLRSTAT_LMIN_SREJ_REG, (byte)1);
	RegValue = I2CBuss.receive();

	if(RegValue == (byte)AS3935_CLRSTAT_LMIN_SREJ_FINAL_CMD)
	{
		Serial.println(F("#AS3935 Sensor Lightning CLRSTAT Verified;"));
	}
	else
	{
		Serial.println(F("#AS3935 Sensor Lightning CLRSTAT is DIFFERENT - ERROR;"));
		return false;
	}

	// Confirm AFE_GB
	I2CBuss.read(SensorAddress, (byte)AS3935_AFE_GB_PWD_REG, (byte)1);
	RegValue = I2CBuss.receive();

	if(RegValue == (byte)AS3935_AFE_GB_PWD_DEFAULT_VALUE)
	{
		Serial.println(F("#AS3935 Sensor Lightning AFE_GB_PWD Verified;"));
	}
	else
	{
		Serial.println(F("#AS3935 Sensor Lightning AFE_GB_PWD is DIFFERENT - ERROR;"));
		return false;
	}

	// Confirm NF_LEV_WDTH
	I2CBuss.read(SensorAddress, (byte)AS3935_NF_LEV_WDTH_REG, (byte)1);
	RegValue = I2CBuss.receive();

	if(RegValue == (byte)AS3935_NF_LEV_WDTH_CMD)
	{
		Serial.println(F("#AS3935 Sensor Lightning NF_LEV_WDTH Verified;"));
	}
	else
	{
		SerialHexBytePrint(RegValue);
		Serial.println(F("#AS3935 Sensor Lightning NF_LEV_WDTH is DIFFERENT - ERROR;"));
		return false;
	}

	if(digitalRead(LightningIRQDigPin) == HIGH)
	{
		Serial.println(F("#AS3935 Sensor Lightning IRQ is HIGH;"));
	}
	else
	{
		Serial.println(F("#AS3935 Sensor Lightning IRQ is LOW;"));
	}

	// Check the value of the look up table first byte
	I2CBuss.read(SensorAddress, (byte)AS3935_LDLUT1_REG, (byte)1);
	RegValue = I2CBuss.receive();

	if(RegValue == (byte)AS3935_LDLUT1_DEFAULT_VALUE)
	{
		Serial.println(F("#AS3935 Sensor Lightning LDLUT1 Verified;"));
	}
	else
	{
		Serial.println(F("#AS3935 Sensor Lightning LDLUT1 is DIFFERENT;"));
		return false;
	}

	I2CBuss.read(SensorAddress, (byte)AS3935_INT_REG, (byte)1);
	RegValue = I2CBuss.receive();

	RegValue = RegValue & AS3935_INT_MASK;

	Serial.print(F("#AS3935 Sensor Lightning INT Value "));
	SerialHexBytePrint(RegValue);
	Serial.println(F(";"));

	if(digitalRead(LightningIRQDigPin) == HIGH)
	{
		Serial.println(F("#AS3935 Sensor Lightning IRQ is HIGH;"));
	}
	else
	{
		Serial.println(F("#AS3935 Sensor Lightning IRQ is LOW;"));
	}

	// Print lookup table
	/*
	for (int i = 0; i < AS3935_NUM_LDLUT_REGS; i++)
	{
		I2CBuss.read(SensorAddress, (byte)(AS3935_LDLUT1_REG + i), (byte)1);
		RegValue = I2CBuss.receive();
		Serial.print(F("#AS3935 Sensor LDLUT  "));
		Serial.print(i+1,DEC);
		Serial.print(F(" Value "));
		SerialHexBytePrint(RegValue);
		Serial.println(F(";"));
	}
	*/

	return true;
}
#endif /*VERIFY_CHIPS*/
