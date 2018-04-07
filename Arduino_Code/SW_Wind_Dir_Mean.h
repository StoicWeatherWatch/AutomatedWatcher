/*
 * SW_Wind_Dir_Mean.h
 *
 *  Created on: 2018-04-07
 *      Author: StoicWeather
 */



#ifndef SW_WIND_DIR_MEAN_H_
#define SW_WIND_DIR_MEAN_H_

#include "SW_Wind_Dir_Analog.h"

#define NUMBER_OF_BINS 16
#define BITS_TO_SHIFT_FOR_BINNING 6
#define HALF_BIN_SIZE 32
#define MAX_ADC_VALUE 1023

// Currently dir every second and speed every 5
//This is defined in SW_Wind_Dir_Mean.h to avoid calloc
#define NUMBER_OF_WIND_DIR_RECORDS 120 // Must be even!

// This is NUMBER_OF_WIND_DIR_RECORDS * 3/2 (An even number is assumed. Otherwise all hell may break loose.)
#define NUMBER_OF_BYTES_NEEDED_FOR_MEAN_DIRECTION_QUEUE  180

//#define RUN_RAM_TEST

//#define PRINT_TEST_LINES

/*
 *
 */
class SW_Wind_Dir_Mean: public SW_Wind_Dir_Analog
{
public:

	byte Bins[NUMBER_OF_BINS];
	byte BinList[NUMBER_OF_BINS];

	/* This is used to hold direction of gust. It is fed to SW_Wind_Dir_Analog
	 * It is created here to avoid a need for calloc since other subclasses of SW_Ard_Analog use different lengths.
	 * */
	byte MeanDirectionQueue[NUMBER_OF_BYTES_NEEDED_FOR_MEAN_DIRECTION_QUEUE];

	SW_Wind_Dir_Mean(byte AAQ0PinIn, byte SensorNumberAnalogIn);


	void SendMeanAndBinBlock();
	void swap(byte *A, byte *B);




};

#endif /* SW_WIND_DIR_MEAN_H_ */
