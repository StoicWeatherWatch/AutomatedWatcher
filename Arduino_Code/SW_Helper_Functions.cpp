/*
 * SW_Helper_Functions.cpp
 *
 *  Created on: 2018-03-21
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
	if(data > (int)255)
	{
		Serial.print(data,HEX);
	}
	else if(data > (int)15)
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
	if(data > (long)0xFFFF)
	{
		Serial.print(data,HEX);
	}
	else if(data > (long)0xFFF)
	{
		Serial.print("0");
		Serial.print(data,HEX);
	}
	else if(data > (long)0xFF)
	{
		Serial.print("00");
		Serial.print(data,HEX);
	}
	else if(data > (long)0xF)
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



