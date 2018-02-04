/*
	Stoic Watcher
	v0.0.8
	2018-02-01
 */


/*
 * This sketch is the core of an Arduino based weather station. An external clock signal at 4 Hz provides timing interrupts.
 * The Arduino reads data from sensors at intervals and transmits it via serial to an attached Linux
 * mini computer running WeeWx.
 *
 * Target Hardware: Arduino Uno R3
 * Output: Serial over USB
 * Input: From sensors and clock square wave
 *
 * Dependencies: I2C library
 */

// TODO Housekeeping, report uptime and chip temperature
// TODO eleminate *3TPH,000000,000000,0000,^; and report an error instead



#include "SW_Conditional_Includes.h"

I2C I2CBus;

SW_BME280_Sensor TPH3_FARS_Sensor = SW_BME280_Sensor((byte)BME280_TPH3_ADDRESS,I2CBus,(byte)BME280_TPH3_TEMPFARS_SNUM);

SW_MCP9808_Sensor T2_CircuitBox_Sensor = SW_MCP9808_Sensor((byte)MCP9808_T2_ADDRESS,I2CBus,(byte)MCP9808_T2_TEMPINBOX_SNUM);

SW_Rain_Readout R4_Rain_Readout = SW_Rain_Readout((byte)RAIN_DAQ0_D_PIN, (byte)RAIN_PIN_RANGE, (byte)RAINCOUNT_RESET_D_PIN, (byte)TIPPINGBUCKET_R4_RAIN_SUNM);

SW_Wind_Dir_Analog W5_WindDir_Readout =  SW_Wind_Dir_Analog((byte)WIND_DIR_ADC_A_PIN, (byte)NUMBER_OF_WIND_DIR_RECORDS, (byte)DAVISANNA_WD5_WIND_DIR_SUNM);

SW_MCP2318_GPIO_Sensor W6_WindSpeed_Sensor = SW_MCP2318_GPIO_Sensor((byte)MCP23018_W6_ADDRESS, I2CBus, (byte)NUMBER_OF_WIND_SPEED_RECORDS_TO_KEEP, (byte)DAVISANNA_WS6_WIND_SPEED_SUNM);

void setup()
{

	// TODO consider 7 data bits (all you need for ASSCI...) https://www.arduino.cc/reference/en/language/functions/communication/serial/begin/
	Serial.begin(SERIAL_BAUDRATE);
	while (!Serial)
	{
		// wait for serial port to connect.
	}

	Serial.println(F("#StoicWatcher Starting v0.0.8;"));
	Serial.println(F("!startup;"));


	I2CBus.begin();
	I2CBus.setSpeed(0);
	I2CBus.timeOut(5000);
	//I2CBus.scan();

	TPH3_FARS_Sensor.InitializeSensor();

	T2_CircuitBox_Sensor.InitializeSensor();

	W6_WindSpeed_Sensor.InitializeSensor();

	// Rain Sensor
	// Set the rain reset high to reset the rain count
	// TODO reevaluate the use of rain reset
	pinMode(RAINCOUNT_RESET_D_PIN, OUTPUT);
	digitalWrite(RAINCOUNT_RESET_D_PIN, HIGH);
	delay(10);
	digitalWrite(RAINCOUNT_RESET_D_PIN, LOW);

	R4_Rain_Readout.setup();

	SW_CK_ClockSetup();

	// THIS IS A TEST BLOCK. IT SHOULD BE REMOVED.
	// TODO REMOVE
	extern int __heap_start, *__brkval;
	int v;
	Serial.print(F("# Free RAM  "));
	Serial.print((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
	Serial.println(F(";"));
	// END TEST



}

void loop()
{
	if(SW_CK_InterruptOccurred())
	{
		// First clock housekeeping
		SW_CK_ClockIntruptProcessing();

		// Every second for wind directions
		if(SW_CK_EverySecond())
		{
			//Serial.println(F("#1 second;"));
			W5_WindDir_Readout.AcquireDataOnly();
			// Every 30 seconds for wind
			if(SW_CK_GetSecondCount() == 0)
			{
				// TODO this prints 3 or 4 times. should run once.
				Serial.println(F("#30 second;"));
				// TODO add sending via serial

			}

		}


		// Regular life cycle
		switch(SW_CK_GetCKShortCount())
		{
		case 0 :
			// 0 Early

			// 0

			break;
		case 1 :
			// 1 Early
			TPH3_FARS_Sensor.AcquireData();

			// 1

			break;
		case 2 :
			// 2 Early

			// 2
			TPH3_FARS_Sensor.RetrieveDataAndSend();

			break;
		case 3 :
			// 3 Early

			// 3

			break;
		case 4 :
			// 4 Early

			// 4

			break;
		case 5 :
			// 5 Early

			// 5

			break;
		case 6 :
			// 6 Early

			// 6
			switch(SW_CK_GetCKMedCount())
			{
			case 0 :
				break;
			case 1 :
				break;
			case 2 :
				break;
			case 3 :
				break;
			}

			// Currently 40 counts or every 90 seconds
			if(SW_CK_GetCKLongCount() == 0)
			{
				T2_CircuitBox_Sensor.AcquireData();
				T2_CircuitBox_Sensor.SendRawDataSerial();
			}

			break;

		case 7 :
				// 7 Early
				R4_Rain_Readout.AcquireDataAndSend();

				// 7

				break;
		case 8 :
				// 8 Early
				W6_WindSpeed_Sensor.AcquireGustDataAndSend();

				// 8

				break;

		}

		// Every second for wind directions
		// TODO does this fire more than once per clock cycle?
		if(SW_CK_GetSubSecondCount() == 0)
		{

			// Every 30 seconds for wind
			if(SW_CK_GetSecondCount() == 0)
			{
				W5_WindDir_Readout.SendQueue();

			}

		}

		// Wind Speed readout.
		if(SW_CK_EveryFifthSecond())
		{
			W6_WindSpeed_Sensor.AcquireData();
			W6_WindSpeed_Sensor.SendAllRawDataSerial();
		}






	} // End if SW_CK_InterruptOccurred


} // main loop


