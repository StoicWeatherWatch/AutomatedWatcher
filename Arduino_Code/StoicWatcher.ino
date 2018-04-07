/*
	Stoic Watcher
	v0.2.3
	2018-04-07
 */
#define PRINT_STOIC_VERSION F("#StoicWatcher Starting v0.2.2;")


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

// TODO Every five seconds will trigger - thus skip certain slower things when long stuff is running





// Get the reset lines right


#include "SW_Conditional_Includes.h"

I2C I2CBus;

SW_BME280_Sensor TPH3_FARS_Sensor = SW_BME280_Sensor((byte)BME280_TPH3_ADDRESS,I2CBus,(byte)BME280_TPH3_TEMPFARS_SNUM);

SW_MCP9808_Sensor T2_CircuitBox_Sensor = SW_MCP9808_Sensor((byte)MCP9808_T2_ADDRESS,I2CBus,(byte)MCP9808_T2_TEMPINBOX_SNUM);

SW_Rain_Readout R4_Rain_Readout = SW_Rain_Readout((byte)RAIN_DAQ0_D_PIN, (byte)RAIN_PIN_RANGE, (byte)RAINCOUNT_RESET_D_PIN, (byte)TIPPINGBUCKET_R4_RAIN_SUNM);

SW_Wind_Dir_Mean W5_WindDir_Mean_Readout =  SW_Wind_Dir_Mean((byte)WIND_DIR_ADC_A_PIN, (byte)NUMBER_OF_WIND_DIR_RECORDS, (byte)DAVISANNA_WD5_WIND_DIR_SUNM);

SW_Wind_Speed_Mean W6_WindSpeed_Mean_Sensor = SW_Wind_Speed_Mean((byte)MCP23018_W6_ADDRESS, I2CBus, (byte)DAVISANNA_WS6_WIND_SPEED_SUNM);

SW_Wind_Gust WG6_WindGust_Multiple = SW_Wind_Gust((byte)MCP23018_W6_ADDRESS, I2CBus, (byte)DAVISANNA_WS6_WIND_SPEED_SUNM, (byte)WIND_DIR_ADC_A_PIN, (byte)DAVISANNA_WD5_WIND_DIR_SUNM);

SW_MCP9808_Sensor T7_FARS_Sensor = SW_MCP9808_Sensor((byte)MCP9808_T7_ADDRESS,I2CBus,(byte)MCP9808_T7_FARSTEMP_SNUM);

SW_MLX90614_Sensor T30_IRSoil_Sensor = SW_MLX90614_Sensor((byte)MLX90614_T30_ADDRESS,I2CBus,(byte)MLX90614_T30_IRTEMP_SNUM);

SW_ChipCap2_Sensor TH8_PRS_Sensor = SW_ChipCap2_Sensor((byte)CHIPCAP2_TH8_ADDRESS,I2CBus,(byte)CHIPCAP2_TH8_PRSTH_SNUM);

SW_BMP280_Sensor TP9_PRS_Sensor = SW_BMP280_Sensor((byte)BMP280_TP9_ADDRESS,I2CBus,(byte)BMP280_TP9_PRSTP_SNUM);
//Not Yet
//SW_SI1133_Sensor EM10_UV_Opt_Sensor = SW_SI1133_Sensor((byte)SI1133_EM10_ADDRESS, I2CBus, (byte)SI1133_EM10_UVOPT_SUNM);

//TEMP
//SW_DS24828_1W_Sensor T20_1Wire_Temp_Sensor = SW_DS24828_1W_Sensor((byte)DS24828_1W_T20_ADDRESS, I2CBus, (byte)DS24828_1W_T20_SNUM);
//Not Yet
//SW_AS3935_Lightning_Sensor EM11_Lightning_Sensor = SW_AS3935_Lightning_Sensor((byte)AS3935_EM11_ADDRESS, I2CBus, (byte)AS3935_EM11_LIGHTNING_SNUM, (byte)LIGHTNING_IRQ_D_PIN);


void setup()
{
	I2CBus.begin();
	I2CBus.setSpeed(0);
	I2CBus.timeOut(5000);

	//There is a 10 ms time limit to start this so it must be done early.

	TH8_PRS_Sensor.InitializeSensor();



	// TODO consider 7 data bits (all you need for ASSCI...) https://www.arduino.cc/reference/en/language/functions/communication/serial/begin/
	Serial.begin(SERIAL_BAUDRATE);
	while (!Serial)
	{
		// wait for serial port to connect.
	}
	delay(10);
	Serial.println(F(""));
	delay(10);
	Serial.println(PRINT_STOIC_VERSION);
	Serial.println(F("!startup;"));
	delay(5);




	//I2CBus.scan();

	TPH3_FARS_Sensor.InitializeSensor();

	T2_CircuitBox_Sensor.InitializeSensor();
	T7_FARS_Sensor.InitializeSensor();


	// Switch this back on for wind
	// TODO Need to reset the wind speed counter on startup (hardware)
	W6_WindSpeed_Mean_Sensor.InitializeSensor();

//Not yet
	//EM10_UV_Opt_Sensor.InitializeSensor();



	// Rain Sensor
	// Set the rain reset high to reset the rain count
	// TODO reevaluate the use of rain reset
// TEMPREARY
	//TODO Switch on the one wire sensor
	//T20_1Wire_Temp_Sensor.InitializeSensor();
	//Not Yet
	//EM11_Lightning_Sensor.InitializeSensor();

	T30_IRSoil_Sensor.InitializeSensor();

	TH8_PRS_Sensor.ReportInitialization();
	TP9_PRS_Sensor.InitializeSensor();


	// Master Reset
	Serial.println(F("# Master Reset;"));
	pinMode(MASTER_RESET_D_PIN, OUTPUT);
	digitalWrite(MASTER_RESET_D_PIN, HIGH);
	delay(10);
	digitalWrite(MASTER_RESET_D_PIN, LOW);
	Serial.println(F("# Master Reset Complete;"));

	// Rain reset?
	//TODO Handle Rain Reset


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


	Serial.flush();
	Serial.println(F("#Setup Done;"));


}

void loop()
{
	/* Time:
	 * Each action in the primary lifecycle gets 100 ms. Each further item in the 1 second and 5 second lifecycles gets 100 ms.
	 * Taking the mean of the wind direction takes about 147 ms (Not good)
	 *
	 * RAM:
	 * 527 bytes are available in wind dir mean
	 * */





	if(SW_CK_InterruptOccurred())
	{
		Serial.println(F("#Stoic SW_CK_InterruptOccurred"));

		// Time test

		//unsigned long time = millis();

		// First clock housekeeping
		SW_CK_ClockIntruptProcessing();
		Serial.println(F("#Stoic SW_CK_ClockIntruptProcessing() done"));

		// Every second for wind directions
		if(SW_CK_EverySecond())
		{
			Serial.println(F("#Stoic calling W5_WindDir_Mean_Readout.AcquireDirectionDataOnly();"));
			W5_WindDir_Mean_Readout.AcquireDirectionDataOnly();
			Serial.println(F("#Stoic W5_WindDir_Mean_Readout.AcquireDirectionDataOnly(); Done"));
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
			T7_FARS_Sensor.AcquireData();

			TH8_PRS_Sensor.SendMeasurmentRequest();
			TP9_PRS_Sensor.AcquireData();




			// 1

			break;
		case 2 :
			// 2 Early

			// 2

			TPH3_FARS_Sensor.RetrieveDataAndSend();
			T7_FARS_Sensor.SendRawDataSerial();

			TH8_PRS_Sensor.PerformDataFetch();



			break;
		case 3 :
			// 3 Early

			// 3
			switch(SW_CK_GetCKMedCount())
						{
						case 0 :
							/* TEMP CUT OUT
							T20_1Wire_Temp_Sensor.Cmd1W_TellDS18B20OnCurrentCHToGetTemp_1W();
							*/

							break;
						case 1 :
							/* TEMP CUT OUT
							T20_1Wire_Temp_Sensor.ReadAndSendRawTempDA18B20OnCurrentCH_1W();
							T20_1Wire_Temp_Sensor.SelectNextChannel();
							*/
							break;
						case 2 :
							break;
						case 3 :
							break;
						}

			break;
		case 4 :
			// 4 Early

			// 4


			 //Switch this back on for wind
			Serial.println(F("#Stoic calling WG6_WindGust_Multiple.AcquireWindGustDirection()"));
			WG6_WindGust_Multiple.AcquireWindGustDirection();

			Serial.println(F("#Stoic WG6_WindGust_Multiple.AcquireWindGustDirection() Done"));


			//WG6_WindGust_Multiple.AcquireAnalogDataAndSend();

			//Not Yet
			//EM11_Lightning_Sensor.CheckIRQ();



			break;
		case 5 :
			// 5 Early



			Serial.println(F("#Stoic calling T30_IRSoil_Sensor.GetTO1DataAndSend();"));
			T30_IRSoil_Sensor.GetTO1DataAndSend();
			Serial.println(F("#Stoic T30_IRSoil_Sensor.GetTO1DataAndSend(); done"));




			// 5
			//Not Yet
			//EM11_Lightning_Sensor.IfIRQGetDataAndSend();

			break;
		case 6 :
			// 6 Early

			Serial.println(F("#Stoic calling TP9_PRS_Sensor.RetrieveDataAndSend();"));
			TP9_PRS_Sensor.RetrieveDataAndSend();
			Serial.println(F("#Stoic TP9_PRS_Sensor.RetrieveDataAndSend(); done"));



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
			// Using 4 not zero so it does not line up with the wind dir mean
			// TODO be more sophisticated about this
			if(SW_CK_GetCKLongCount() == 4)
			{

				T2_CircuitBox_Sensor.AcquireData();
				T2_CircuitBox_Sensor.SendRawDataSerial();

			}



			break;

		case 7 :
				// 7 Early
			Serial.println(F("#Stoic calling R4_Rain_Readout.AcquireDataAndSend();"));
				R4_Rain_Readout.AcquireDataAndSend();
				Serial.println(F("#Stoic  R4_Rain_Readout.AcquireDataAndSend(); done"));




				// 7

				break;
		case 8 :
				// 8 Early

			Serial.println(F("#Stoic calling WG6_WindGust_Multiple.AcquireWindGustSpeed();"));
				WG6_WindGust_Multiple.AcquireWindGustSpeed();

				WG6_WindGust_Multiple.SendWindGustData();
				Serial.println(F("#Stoic  WG6_WindGust_Multiple.AcquireWindGustSpeed(); done"));


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
				// TODO fix Wind Dir
				//W5_WindDir_Mean_Readout.SendDirectionQueue();

			}

		}

		// Wind Speed readout.
		if(SW_CK_EveryFifthSecond())
		{

			Serial.println(F("#Stoic SW_CK_EveryFifthSecond"));
			W6_WindSpeed_Mean_Sensor.AcquireData();

			W6_WindSpeed_Mean_Sensor.SendMostRecentRawMean();

			W5_WindDir_Mean_Readout.SendMeanAndBinBlock();

// TODO Wind mean direction prints before speed starts to. Why? Maybe zero speed? Might be fixed. Changed speed records from 30 to 24

		}


		// Time test

		/*Serial.flush();
			unsigned long Endtime = millis();
			Serial.print(F("# Time for this cycle "));
			Serial.print(Endtime-time);
			Serial.println(F(";"));*/


		Serial.println(F("#Stoic End if SW_CK_InterruptOccurred"));

	} // End if SW_CK_InterruptOccurred

	Serial.println(F("#Stoic Loop done"));
} // main loop


