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


// Addresses
#define BME280_TPH1_ADDRESS 0x77
#define MCP9808_T2_ADDRESS 0x18

// Life Cycle Controls
#define BME280_TPH1_TAKEMEASURE_LCS 1
#define BME280_TPH1_READMEASURE_LCS 2

#define MCP9808_T2_READMEASURE_LCS  1

// Sensor Numbers
#define BME280_TPH1_TEMPFARS_SNUM 1
#define MCP9808_T2_TEMPINBOX_SNUM 2



#endif /* SW_CONDITIONAL_INCLUDES_H_ */
