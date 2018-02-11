/*
 * SW_Helper_Functions.cpp
 *
 *  Created on: 2018-02-06
 *      Author: StoicWeather
 */

#include "SW_Helper_Functions.h"

// prints 8-bit data in hex with leading zeroes
bool SerialHexBytePrint(byte data)
{
	if(data > (byte)15)
	{
		Serial.print(data,HEX);
	}
	else
	{
		Serial.print("0");
		Serial.print(data,HEX);
	}

	return true;
}

// prints 12-bit data in hex with leading zeroes
bool SerialHexByteAndAHalfPrint(int data)
{
	if(data > (byte)255)
	{
		Serial.print(data,HEX);
	}
	else if(data > (byte)15)
	{
		Serial.print("0");
		Serial.print(data,HEX);
	}
	else
	{
		Serial.print("00");
		Serial.print(data,HEX);
	}

	return true;
}

void SerialHexFourAndAHalfBytefPrint(long data)
{
	if(data > (byte)0xFFFF)
	{
		Serial.print(data,HEX);
	}
	else if(data > (byte)0xFFF)
	{
		Serial.print("0");
		Serial.print(data,HEX);
	}
	else if(data > (byte)0xFF)
	{
		Serial.print("00");
		Serial.print(data,HEX);
	}
	else if(data > (byte)0xF)
	{
		Serial.print("000");
		Serial.print(data,HEX);
	}
	else
	{
		Serial.print("0000");
		Serial.print(data,HEX);
	}

}



