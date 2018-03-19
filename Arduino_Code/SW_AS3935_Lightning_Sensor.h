/*
 * SW_AS3935_Lightning_Sensor.h
 *
 *  Created on: 2018-02-22
 *      Author: StoicWeather
 */

#ifndef SW_AS3935_LIGHTNING_SENSOR_H_
#define SW_AS3935_LIGHTNING_SENSOR_H_

#include "SWSensor.h"
#include "SW_Helper_Functions.h"

#define SEND_MEANINGLESS_ENERGY

#define AS3935_TUNE_CAP_VALUE 6

#define AS3935_TUNECAP_IRQ_CONTROL_REG      0x08
#define AS3935_CLRSTAT_LMIN_SREJ_REG        0x02
#define AS3935_NF_LEV_WDTH_REG              0x01
#define AS3935_AFE_GB_PWD_REG               0x00
#define AS3935_PRESET_DEFAULT_REG           0x3C
// Calibrates automatically the internal RC Oscillators
#define AS3935_CALIB_RCO_REG                0x3D

#define AS3935_INT_REG                      0x03
#define AS3935_LIGHTNING_ENERGY_LSB_REG     0x04
#define AS3935_LIGHTNING_ENERGY_MOREB_REG   0x05
#define AS3935_LIGHTNING_ENERGY_MMSB_REG    0x06
#define AS3935_LIGHTNING_DISTANCE_REG       0x07


// Used with AS3935_PRESET_DEFAULT_REG and AS3935_CALIB_RCO_REG
#define AS3935_EXE_ACTION_CMD                     0x96
// Per Embedded Adventures, Tune Cap value is 6. The sticker only says 6.
// Upper bits should be zero to avoid chaos in the universe.
#define AS3935_TUNECAP_IRQ_CONTROL_CMD                   0x06
// Used for calibrating RCO. See data sheet page 23
#define AS3935_TUNECAP_IRQ_CONTROL_TRCO_ON_IRQ_CMD       0x16

//It is possible to clear the statistics built up by the lightning distance estimation algorithm block by just toggling the bit REG0x02[6] (high-low-high).
// Everything except bit 6 should be kept the same for these
// Lowest 4 bits are SREJ the signal rejection threshold.  First trying 0001 not the default of 0010
#define AS3935_CLRSTAT_LMIN_SREJ_TOGGEL_CMD       0x81
#define AS3935_CLRSTAT_LMIN_SREJ_FINAL_CMD        0xC1
#define AS3935_NF_LEV_WDTH_CMD                    0x21
// AFE sets the indoor or outdoor mode
#define AS3935_AFE_GB_PWD_OUTDOOR               0x1C
#define AS3935_AFE_GB_PWD_INDOOR                0x24
#define AS3935_AFE_GB_PWD_CMD               AS3935_AFE_GB_PWD_INDOOR

// For values of INT AS3935_INT_REG bits 3-0
//Noise level too high
#define AS3935_INT_NH_VALUE    0b00000001
//Disturber detected
#define AS3935_INT_D_VALUE     0b00000100
//Lightning interrupt
#define AS3935_INT_L_VALUE     0b00001000
//Lightning interrupt - Distance changed due to purging data
#define AS3935_INT_L0_VALUE    0b00000000

#define AS3935_INT_MASK 	 	                  0b00001111
#define AS3935_LIGHTNING_ENERGY_MMSB_MASK 	  0b00011111
#define AS3935_LIGHTNING_DISTANCE_MASK   	  0b00111111

#ifdef VERIFY_CHIPS
#define AS3935_LDLUT1_REG           0x09
#define AS3935_LDLUT1_DEFAULT_VALUE           0b10101101

#define AS3935_NUM_LDLUT_REGS       42

#define AS3935_AFE_GB_PWD_DEFAULT_VALUE           AS3935_AFE_GB_PWD_CMD
#endif /*VERIFY_CHIPS*/

/*
 *
 */
class SW_AS3935_Lightning_Sensor: public SW_Sensor
{
public:
	byte LightningIRQDigPin;
	bool IRQReceived;

	SW_AS3935_Lightning_Sensor(byte AddressIn, I2C I2CBussIn, byte SensorNumberIN, byte LightningIRQDigPinIn);

	bool InitializeSensor();

	void CheckIRQ();
	void IfIRQGetDataAndSend();

	void GetAndSendData();

#ifdef VERIFY_CHIPS
	bool VerifyChip();
#endif /*VERIFY_CHIPS*/
};

#endif /* SW_AS3935_LIGHTNING_SENSOR_H_ */
