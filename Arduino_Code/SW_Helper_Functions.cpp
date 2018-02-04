/*
 * SW_Helper_Functions.cpp
 *
 *  Created on: 2018-01-25
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

