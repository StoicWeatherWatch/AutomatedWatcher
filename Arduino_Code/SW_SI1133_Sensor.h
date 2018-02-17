/*
 * SW_SI1133_Sensor.h
 *
 *  Created on: 2018-02-13
 *      Author: StoicWeather
 *
 *      The SI1133 needs 25 ms to start up before I2C activity.
 */

#ifndef SW_SI1133_SENSOR_H_
#define SW_SI1133_SENSOR_H_

#include "SWSensor.h"

#include "SW_Helper_Functions.h"

#define SI1133_CHIPID_REG     0x00
#define SI1133_REVID_REG      0x01
#define SI1133_MFRID_REG      0x02


#define SI1133_CHIPID_VALUE   0x33

/*
 *
 */
class SW_SI1133_Sensor: public SW_Sensor
{
public:
	SW_SI1133_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);



#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/

};

#endif /* SW_SI1133_SENSOR_H_ */
