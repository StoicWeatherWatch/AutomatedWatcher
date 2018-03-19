/*
 * SW_Conditional_Includes.h
 *
 *  Created on: 2018-02-08
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

#include "SW_Helper_Functions.h"
#include "SW_Clock.h"

#include "SWSensor.h"
#include "SW_BME280_Sensor.h"
#include "SW_MCP9808_Sensor.h"
#include "SW_MCP2318_GPIO_Sensor.h"
#include "SW_Wind_Speed_Mean.h"
#include "SW_SI1133_Sensor.h"
#include "SW_DS24828_1W_Sensor.h"
#include "SW_AS3935_Lightning_Sensor.h"
#include "SW_ChipCap2_Sensor.h"
#include "SW_MLX90614_Sensor.h"
#include "SW_BMP280_Sensor.h"

#include "SW_Ard_Readout.h"
#include "SW_Rain_Readout.h"

#include "SW_Ard_Analog.h"
#include "SW_Wind_Dir_Analog.h"
#include "SW_Wind_Dir_Mean.h"

#include "SW_Wind_Gust.h"


#define SERIAL_BAUDRATE 9600


// Addresses
#define BME280_TPH3_ADDRESS     0x77
#define MCP9808_T2_ADDRESS      0x18
// Address 32 or 0x20 in HEX
#define MCP23018_W6_ADDRESS     0x20
#define MCP9808_T7_ADDRESS      0x19
#define SI1133_EM10_ADDRESS     0x55
#define DS24828_1W_T20_ADDRESS  0x1F
#define AS3935_EM11_ADDRESS     0x03
#define MLX90614_T29_ADDRESS    0x5A
#define CHIPCAP2_TH8_ADDRESS    0x28
#define BMP280_TP9_ADDRESS      0x76


// Sensor Numbers
#define MCP9808_T2_TEMPINBOX_SNUM 		2
#define BME280_TPH3_TEMPFARS_SNUM 		3
#define TIPPINGBUCKET_R4_RAIN_SUNM 		4
#define DAVISANNA_WD5_WIND_DIR_SUNM 		5
#define DAVISANNA_WS6_WIND_SPEED_SUNM 	6
#define MCP9808_T7_FARSTEMP_SNUM 		7
#define CHIPCAP2_TH8_PRSTH_SNUM 		    8
#define BMP280_TP9_PRSTP_SNUM            9
#define SI1133_EM10_UVOPT_SUNM          10
#define AS3935_EM11_LIGHTNING_SNUM      11
#define DS24828_1W_T20_SNUM             20
#define MLX90614_T29_IRTEMP_SNUM 	   29


// Pins
#define RAINCOUNT_RESET_D_PIN  13
#define MASTER_RESET_D_PIN      8
// Rain 9-12 See RAIN_PIN_RANGE
#define RAIN_DAQ0_D_PIN         9
#define LIGHTNING_IRQ_D_PIN     4

// Analog pins
#define WIND_DIR_ADC_A_PIN      1


// Pin Ranges - number of pins starting with DAQ0
#define RAIN_PIN_RANGE     4

// Presets
// Currently dir every second and speed every 5
#define NUMBER_OF_WIND_DIR_RECORDS 120 // Must be even!
#define NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP   24  // Should match the above as related to the life cycle.
// 2.25 seconds gives 54 in 121.5 seconds
#define NUMBER_OF_WIND_GUST_RECORDS_TO_KEEP   54



#endif /* SW_CONDITIONAL_INCLUDES_H_ */
