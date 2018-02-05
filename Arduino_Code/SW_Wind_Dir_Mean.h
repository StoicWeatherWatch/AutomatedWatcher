/*
 * SW_Wind_Dir_Mean.h
 *
 *  Created on: 2018-02-04
 *      Author: StoicWeather
 */



#ifndef SW_WIND_DIR_MEAN_H_
#define SW_WIND_DIR_MEAN_H_

#include "SW_Wind_Dir_Analog.h"

#define NUMBER_OF_BINS 16
#define BITS_TO_SHIFT_FOR_BINNING 6
#define HALF_BIN_SIZE 32
#define MAX_ADC_VALUE 1023

#define RUN_RAM_TEST

/*
 *
 */
class SW_Wind_Dir_Mean: public SW_Wind_Dir_Analog
{
public:
	SW_Wind_Dir_Mean(byte AAQ0PinIn, byte NumberOfRecordsIn, byte SensorNumberAnalogIn);


	void SendMeanAndBinBlock();
	void swap(byte *A, byte *B);



};

#endif /* SW_WIND_DIR_MEAN_H_ */
