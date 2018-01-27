/*
 * SW_Helper_Functions.cpp
 *
 *  Created on: 2018-01-25
 *      Author: StoicWeather
 */

#include "SW_Helper_Functions.h"


bool SerialHexBytePrint(byte data) // prints 8-bit data in hex with leading zeroes
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

