/*
 * SW_MCP2318_GPIO_Sensor.cpp
 *
 *  Created on: 2018-02-06
 *      Author: StoicWeather
 *
 */

/*
 * NWS Wind gust rapid fluctuations of wind speed with a variation of 10 knots or more between peaks and lulls.
 * The gust speed is the max speed recorded.
 */

#include "SW_MCP2318_GPIO_Sensor.h"

SW_MCP2318_GPIO_Sensor::SW_MCP2318_GPIO_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN)
:SW_Sensor(AddressIn,I2CBussIn, SensorNumberIN)
{


}

int SW_MCP2318_GPIO_Sensor::AcquireDataAndReturn()
{
#ifdef SET_FULL_TESTS
	Serial.println(F("# MCP2318 AcquireDataAndReturn;"));
#endif /*SET_FULL_TESTS*/
	//SEQOP = 0 So the second byte should be GPIOA :)
#ifdef SET_FULL_TESTS
	byte status = I2CBuss.read(SensorAddress, (byte)MCP2318_GPIOB_REG, (byte)1);
#endif /*SET_FULL_TESTS*/
#ifndef SET_FULL_TESTS
	I2CBuss.read(SensorAddress, (byte)MCP2318_GPIOB_REG, (byte)1);
#endif /*SET_FULL_TESTS*/

#ifdef SET_FULL_TESTS
	Serial.print(F("# MCP2318 read status "));
	Serial.print(status,HEX);
	Serial.println(F(";"));

	//TEST line
	Serial.print(F("# MCP2318 I2c.available()  "));
	Serial.print(I2CBuss.available(),HEX);
	Serial.println(F(";"));
	//TEST line
	Serial.print(F("# MCP2318 readin "));
#endif /*SET_FULL_TESTS*/

	int DataIn = (int)I2CBuss.receive();

#ifdef SET_FULL_TESTS
	Serial.print(DataIn, HEX);
	Serial.print(F("   "));
#endif /*SET_FULL_TESTS*/
#ifdef SET_REPORT_BIT_READ
	Serial.print(F("# MCP2318 MSB "));
	Serial.print(DataIn,BIN);
	Serial.println(F(" ;"));
#endif /*SET_REPORT_BIT_READ*/

	I2CBuss.read(SensorAddress, (byte)MCP2318_GPIOA_REG, (byte)1);

	// Most sig bits came in first. They get shifted up. Only 13 bits of data.
	DataIn = DataIn &0b00011111;
	DataIn <<= 8;

#ifdef SET_FULL_TESTS
	//int temp = (int)I2CBuss.receive();
	//Serial.print(temp, HEX);
	//Serial.println(F(";"));
	//DataIn |= temp;
#endif /*SET_FULL_TESTS*/
#ifdef SET_REPORT_BIT_READ
	int temp = (int)I2CBuss.receive();

	Serial.print(F("# MCP2318 LSB "));
	Serial.print(temp,BIN);
	Serial.println(F(" ;"));

	DataIn |= temp;
#endif /*SET_REPORT_BIT_READ*/
#ifndef SET_REPORT_BIT_READ
	DataIn |= (int)I2CBuss.receive();
#endif /*SET_REPORT_BIT_READ*/

#ifdef SET_REPORT_BIT_READ
	Serial.print(F("# MCP2318 ALL "));
	Serial.print(DataIn,BIN);
	Serial.println(F(" ;"));
	Serial.print(F("# MCP2318 ALL "));
		Serial.print(DataIn,HEX);
		Serial.println(F(" ;"));
#endif /*SET_REPORT_BIT_READ*/



#ifdef REPORT_ALL_READINGS_TEST
	Serial.print(F("# MCP23018 readin "));
	Serial.print(DataIn, BIN);
	Serial.println(F(";"));
#endif /*REPORT_ALL_READINGS_TEST*/

#ifdef SET_FULL_TESTS
	Serial.println(F("# AcquireDataAndReturn Done returning"));
#endif /*SET_FULL_TESTS*/

#ifdef REPORT_MEMORY_LEVEL
extern int __heap_start, *__brkval;
	int v;
	Serial.print(F("# Free RAM  "));
	Serial.print((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
	Serial.println(F(";"));
	// END TEST
#endif /*REPORT_MEMORY_LEVEL*/

	return DataIn;
}



/*bool SW_MCP2318_GPIO_Sensor::SendAllRawDataSerial()
{

	if(!HaveFullSpeedQueue)
	{

		return false;
	}

	Serial.print(F("#"));
	Serial.print(SensorNumber,DEC);
	Serial.print(F("WSA"));
	byte QueueLoc = 0;
	for(int i = 0; i< SpeedQueueLength; i++)
	{
		QueueLoc = CurrentSpeedQueueLoc + i;

		// TODO CurrentQueueLoc >QueueLength >=?
		if(QueueLoc >= SpeedQueueLength)
		{
			QueueLoc -= SpeedQueueLength;
		}

		Serial.print(F(","));
		SerialHexBytePrint(WindSpeedQueue[QueueLoc*2]);
		SerialHexBytePrint(WindSpeedQueue[(QueueLoc*2)+1]);

	}

	Serial.print(F(";"));

	return true;



}

// todo fix this
bool SW_MCP2318_GPIO_Sensor::SendRawDataSerial()
{
	return SendAllRawDataSerial();
}*/




// A gust is 10 knts between peak and lull in a time period that is rapid.
//Readout is every 2.25 seconds for instantaneous speed.
// How many 2.25 second intervals is set by gust Record Interval
//bool SW_MCP2318_GPIO_Sensor::AcquireGustData();
// THis sends raw counts from a 2.25 second period. Sends max, min, and current for
// Time period recorded.
//bool SW_MCP2318_GPIO_Sensor::SendGustData();



bool SW_MCP2318_GPIO_Sensor::InitializeSensor()
{
	Serial.println(F("#Initializing MCP23018;"));

#ifdef VERIFY_CHIPS
	if(!VerifyChip())
	{
		return false;
	}
#endif /*VERIFY_CHIPS*/

	// Set configuration parameters
	// Only needed if we change from defaults
	//I2CBuss.write(SensorAddress, (byte)MCP2318_IOCON_REG, (byte)MCP2318_CONFIG_IOCON_CMD);

	// Only need to set IODIR if using output.



	return true;

}



#ifdef VERIFY_CHIPS
bool SW_MCP2318_GPIO_Sensor::VerifyChip()
{
	// There are no IDs baked in. We verify the default startup conditions

	// Check IOCON default
	byte status = I2CBuss.read(SensorAddress, (byte)MCP2318_IOCON_REG, (byte)2);

	//TEST line
	//Serial.print(F("# MCP2318 I2c.available()  "));
	//Serial.print(I2CBuss.available(),HEX);
	//Serial.println(F(";"));

	int DataIn = (int)I2CBuss.receive();
	// Most sig bits came in first. They get shifted up.
	DataIn <<= 8;
	DataIn |= (int)I2CBuss.receive();

	//TEST line
	Serial.print(F("# MCP23018 VerifyChip read status "));
	Serial.print(status,HEX);
	Serial.println(F(";"));


	if(DataIn == (int)MCP2318_IOCON_DEFAULT_VALUE)
	{
		Serial.println(F("#MCP23018 Correct IOCON Values;"));
	}
	else
	{
		Serial.println(F("#NOT MCP23018 Correct IOCON Values;"));
		return false;
	}

	// Check IODIR default
	status = I2CBuss.read(SensorAddress, (byte)MCP2318_IODIRA_REG, (byte)2);


	//TEST line
	//Serial.print(F("# MCP23018 I2c.available()  "));
	//Serial.print(I2CBuss.available(),HEX);
	//Serial.println(F(";"));

	DataIn = (int)I2CBuss.receive();
	// Most sig bits came in first. They get shifted up.
	DataIn <<= 8;
	DataIn |= (int)I2CBuss.receive();

	//TEST line
	//Serial.print(F("# MCP2318 VerifyChip read status "));
	//Serial.print(status,HEX);
	//Serial.println(F(";"));


	if(DataIn == (int)MCP2318_IODIRA_DEFAULT_VALUE)
	{
		Serial.println(F("#MCP23018 Correct IODIR Values;"));
	}
	else
	{
		Serial.println(F("#NOT MCP23018 Correct IODIR Values;"));
		Serial.println(DataIn);
		return false;
	}


	return true;

}
#endif /*VERIFY_CHIPS*/
