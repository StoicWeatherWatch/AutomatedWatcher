/*
	Stoic Watcher
	v0.0.5
	2018-01-25
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

SW_BME280_Sensor TPH3_FARS_Sensor = SW_BME280_Sensor((byte)BME280_TPH3_ADDRESS,I2CBus,(byte)BME280_TPH3_TEMPFARS_SNUM);

SW_MCP9808_Sensor T2_CircuitBox_Sensor = SW_MCP9808_Sensor((byte)MCP9808_T2_ADDRESS,I2CBus,(byte)MCP9808_T2_TEMPINBOX_SNUM);

SW_Rain_Readout R4_Rain_Readout = SW_Rain_Readout((byte)RAIN_DAQ0_D_PIN, (byte)RAIN_PIN_RANGE, (byte)MASTER_RESET_D_PIN, (byte)TippingBucket_R4_Rain_SUNM);



void setup()
{

  Serial.begin(SERIAL_BAUDRATE);
  while (!Serial)
  {
     // wait for serial port to connect.
  }

  Serial.println("#Stoic Starting v0.0.5;");
  Serial.println("!startup;");

  //SerialHexBytePrint((byte)65);
  Serial.println("");


  I2CBus.begin();

  I2CBus.setSpeed(0);
  I2CBus.timeOut(5000);

  //I2CBus.scan();

  TPH3_FARS_Sensor.InitializeSensor();

  T2_CircuitBox_Sensor.InitializeSensor();

  SW_CK_ClockSetup();

  // Set the master reset high to reset everything
  pinMode(MASTER_RESET_D_PIN, OUTPUT);
  digitalWrite(MASTER_RESET_D_PIN, HIGH);
  delay(10);
  digitalWrite(MASTER_RESET_D_PIN, LOW);


  R4_Rain_Readout.setup();


}

void loop()
{
	if(SW_CK_InterruptOccurred())
	{
		SW_CK_ClockIntruptProcessing();
		Serial.println("#Clock Interrupt;");
		SW_CK_SendLongCountSerial();

		T2_CircuitBox_Sensor.AcquireData();
		//Serial.println(T2_CircuitBox_Sensor.GetRawTempreature_HighBits(),BIN);
		//Serial.println(T2_CircuitBox_Sensor.GetRawTempreature_LowBits(),BIN);
		//Serial.print("");
		Serial.print("#");
		Serial.print(T2_CircuitBox_Sensor.ProcessTemp());
		Serial.println(";");

		T2_CircuitBox_Sensor.SendRawDataSerial();

		if(SW_CK_GetCKShortCount() == BME280_TPH3_TAKEMEASURE_LCS)
		{
			TPH3_FARS_Sensor.AcquireData();
		}

		if(SW_CK_GetCKShortCount() == BME280_TPH3_READMEASURE_LCS)
		{
			TPH3_FARS_Sensor.RetrieveDataAndSend();
		}

		R4_Rain_Readout.AcquireDataAndSend();



	} // End if SW_CK_InterruptOccurred




} // main loop


