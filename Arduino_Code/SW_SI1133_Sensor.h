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

#define SI1133_PARTID_REG        0x00
#define SI1133_REVID_REG         0x01
#define SI1133_MFRID_REG         0x02
#define SI1133_INFO0_REG         0x03
#define SI1133_INFO1_REG         0x04
#define SI1133_COMMAND_REG       0x0B
#define SI1133_RESPONSE_0_REG    0x10
#define SI1133_RESPONSE_1_REG    0x11
#define SI1133_HOSTIN_0_REG      0x0A





#define SI1133_RESET_CMD_CTR_CMD           0x00
#define SI1133_RESET_SW_CMD                0x01
#define SI1133_FORCE_MEASURMENT_CMD        0x11

#define SI1133_CHIPID_VALUE                    0x33
#define SI1133_RESPONSE0_ON_RESET_VALUE        0b00001111
#define SI1133_RESPONSE0_ON_CMD_ERR_VALUE      0b00010000

#define SI1133_CMD_ERR_CODE_SAT_OVERFL_VALUE   0x12

#define SI1133_RESPONSE0_ON_RESET_MASK         0b00011111
#define SI1133_RESPONSE0_CMD_ERR_MASK          0b00010000
#define SI1133_RESPONSE0_CMD_ERR_CODE_MASK     0b00001111


// TODO Per page 3, handle high and low light by adjusting integration time
// TODO per page 3 bottom subtract dark current

// TODO double read measurments to varify


/*
 *
 */
class SW_SI1133_Sensor: public SW_Sensor
{
public:
	SW_SI1133_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN);

	bool InitializeSensor();

	bool CheckForSaturation();
	bool CheckForCMDError();




#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/

};

#endif /* SW_SI1133_SENSOR_H_ */
