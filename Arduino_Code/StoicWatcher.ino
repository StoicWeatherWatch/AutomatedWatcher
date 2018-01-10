/*
	Stoic Watcher
	v0.0.0
	2018-01-09
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



void setup()
{

  Serial.begin(9600);
  while (!Serial)
  {
     // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("");



}

void loop()
{

}


