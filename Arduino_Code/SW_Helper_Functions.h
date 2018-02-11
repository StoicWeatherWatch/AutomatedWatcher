/*
 * SW_Helper_Functions.h
 *
 *  Created on: 2018-01-27
 *      Author: StoicWeather
 */

#ifndef SW_HELPER_FUNCTIONS_H_
#define SW_HELPER_FUNCTIONS_H_

#include "Arduino.h"

// prints 8-bit data in hex with leading zeroes
// Uses print not println
bool SerialHexBytePrint(byte data);
bool SerialHexByteAndAHalfPrint(int data);
void SerialHexFourAndAHalfBytefPrint(long data);


#endif /* SW_HELPER_FUNCTIONS_H_ */
