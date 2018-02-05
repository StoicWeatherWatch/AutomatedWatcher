/*
 * SW_Wind_Dir_Mean.h
 *
 *  Created on: 2018-02-04
 *      Author: StoicWeather
 */

#ifndef SW_WIND_DIR_MEAN_H_
#define SW_WIND_DIR_MEAN_H_

#include "SW_Wind_Dir_Analog.h"

/*
 *
 */
class SW_Wind_Dir_Mean: public SW_Wind_Dir_Analog
{
public:
	SW_Wind_Dir_Mean(byte AAQ0PinIn, byte NumberOfRecordsIn, byte SensorNumberAnalogIn);
};

#endif /* SW_WIND_DIR_MEAN_H_ */
