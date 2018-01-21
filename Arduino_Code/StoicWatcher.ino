/*
	Stoic Watcher
	v0.0.1
	2018-01-21
 */


/*
 * This sketch is the core of an Arduino based weather station. An external RTC provides timing interrupts.
 * The Arduino reads data from sensors at intervals and transmits it via serial to an attached Linux
 * mini computer running WeeWx.
 *
 * Target Hardware: Arduino Uno R3
 * Output: Serial over USB
 * Input: From sensors and RTC square wave
 */


#include "SW_Conditional_Includes.h"

I2C I2CBus;

SW_BME280_Sensor TPH1_FARS_Sensor = SW_BME280_Sensor((byte)BME280_TPH1_ADDRESS,I2CBus);

SW_MCP9808_Sensor T2_CircuitBox_Sensor = SW_MCP9808_Sensor((byte)MCP9808_T1_ADDRESS,I2CBus);



void setup()
{

  Serial.begin(9600);
  while (!Serial)
  {
     // wait for serial port to connect.
  }

  Serial.println("#Stoic Starting v0.0.1;");
  Serial.println("!startup;");



  I2CBus.begin();

  I2CBus.setSpeed(0);
  I2CBus.timeOut(5000);

  //I2CBus.scan();

  TPH1_FARS_Sensor.InitializeSensor();

  T2_CircuitBox_Sensor.InitializeSensor();

  SW_CK_ClockSetup();


}

void loop()
{
	if(SW_CK_InterruptOccurred())
	{
		SW_CK_ClockIntruptProcessing();
		Serial.println("#Clock Interrupt;");
		Serial.println(SW_CK_GetCKLongCount());

	}




}


