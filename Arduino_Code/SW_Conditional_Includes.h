/*
 * SW_Conditional_Includes.h
 *
 *  Created on: 2018-01-21
 *      Author: StoicWeather
 */

#ifndef SW_CONDITIONAL_INCLUDES_H_
#define SW_CONDITIONAL_INCLUDES_H_

#include "Arduino.h"

/* Uses I2C Rev 5 Library from Wayne Truchsess
 * Posted at
 * http://dsscircuits.com/articles/arduino-i2c-master-library
 * https://github.com/rambo/I2C
 * GNU GPL
 */
#include "I2C.h"


#include "SW_Clock.h"

#include "SWSensor.h"
#include "SW_BME280_Sensor.h"
#include "SW_MCP9808_Sensor.h"

#include "SW_Ard_Readout.h"
#include "SW_Rain_Readout.h"

#define SERIAL_BAUDRATE 9600


// Addresses
#define BME280_TPH3_ADDRESS 0x77
#define MCP9808_T2_ADDRESS 0x18

// Life Cycle Controls
#define BME280_TPH3_TAKEMEASURE_LCS 1
#define BME280_TPH3_READMEASURE_LCS 2
// TODO should it be LCS or MCS?

#define MCP9808_T2_READMEASURE_LCS  1

// Sensor Numbers
#define BME280_TPH3_TEMPFARS_SNUM 3
#define MCP9808_T2_TEMPINBOX_SNUM 2
#define TippingBucket_R4_Rain_SUNM 4

// Pins
#define MASTER_RESET_D_PIN 13
#define RAIN_DAQ0_D_PIN    9

// Pin Ranges - number of pins starting with DAQ0
#define RAIN_PIN_RANGE     4



#endif /* SW_CONDITIONAL_INCLUDES_H_ */
