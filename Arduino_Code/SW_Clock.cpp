/*
 * SW_Clock.cpp
 *
 *  Created on: 2018-01-21
 *      Author: StoicWeather
 */

#include "SW_Clock.h"

#if(ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

// TODO consider returning pointers from get functions. It would save memory.
// TODO change get functions to match byte or int not just int.

// For some reason this cannot go in the .h file
bool SW_CK_CKInterrupted = false;

// An interrupt is received at a pin every 0.25 s or 4 Hz

// Short is every 9 clock pulses or 2.25 s
byte SW_CK_CKShortCountTo9 = 0;

// Counts every 9th short to count in increments of 2.25 s
// Reset every 4 or every 9 seconds
byte SW_CK_CKMedCount = 0;

// Counts every 9th short to count in increments of 2.25 s
// Reset every 40 or every 90 seconds or 360 interrupts
int SW_CK_CKLongCount = 0;

// Counts every 90s interval. 2 for 3 min, 40 for 1 hour. 960 for 1 day
int SW_CK_CK90sCount = 0;

// Counts every second to 30
byte SW_CK_SecondCount = 0;

// Counts every second to 5
byte SW_CK_FiveSecondCount = 0;

// Counts every clock pulse to 4. Needed for second count
byte SW_CK_CKShortCountTo4 = 0;

bool SW_CK_EveryFifthSecondNotCalled = false;
bool SW_CK_EverySecondNotCalled = false;



bool SW_CK_ClockSetup()
{
	attachInterrupt(digitalPinToInterrupt(SW_CK_CLOCK_PIN), SW_CK_ClockInterrupt,RISING);

	Serial.println("#Clock Interrupt Starting;");

	while(!SW_CK_CKInterrupted)
	{
		// Wait for clock. Need to have something here. Empty did not work.
		delay(100);
	}

	Serial.println("#Clock Interrupt Functional;");
	SW_CK_CKInterrupted = false;

	return true;
}

// Attached to Interrupt pin
void SW_CK_ClockInterrupt()
{
	SW_CK_CKInterrupted = true;
    //Serial.println("#ClockIntrupt;");
}

void SW_CK_ClockIntruptProcessing()
{
	SW_CK_CKInterrupted = false;

	SW_CK_CKShortCountTo9++;
	SW_CK_CKShortCountTo4++;

	if(SW_CK_CKShortCountTo9 >= SW_CK_SHORT_COUNT_PERIOD)
	{
		SW_CK_CKShortCountTo9 = 0;

		SW_CK_CKMedCount++;
		if(SW_CK_CKMedCount >= SW_CK_MED_COUNT_PERIOD)
		{
			SW_CK_CKMedCount = 0;
		}

		SW_CK_CKLongCount++;
		if(SW_CK_CKLongCount >= SW_CK_LONG_COUNT_PERIOD)
		{
			SW_CK_CKLongCount = 0;

			SW_CK_CK90sCount++;

#ifdef SW_CK_REPORT_UP_TIME
#ifdef SW_CK_REPORT_UP_TIME_MACHINE_READABLE
			Serial.print(F("*AUT,"));
			Serial.print(SW_CK_CK90sCount);
			Serial.println(F(";"));
#else /*SW_CK_REPORT_UP_TIME_MACHINE_READABLE - Print as comment*/
			Serial.print(F("#Uptime,"));
			int Tminutes = (SW_CK_CK90sCount * 2) / 3;
			Serial.print(Tminutes);
			Serial.println(F(",minutes;"));
#endif /*SW_CK_REPORT_UP_TIME_MACHINE_READABLE*/
#endif /* SW_CK_REPORT_UP_TIME */

		}
	} // Short Count to 9 updater

	// Handle Seconds count
	if(SW_CK_CKShortCountTo4 >= 4)
	{
		//Serial.println(F("#SW_CK_CKShortCountTo4 >= 4;"));
		SW_CK_SecondCount++;
		SW_CK_FiveSecondCount++;
		SW_CK_CKShortCountTo4 = 0;

		// Makes certain Every second returns true only once
		SW_CK_EverySecondNotCalled = true;

		if(SW_CK_FiveSecondCount >= 5)
				{
			SW_CK_FiveSecondCount = 0;
			SW_CK_EverySecondNotCalled = true;

				}



		if(SW_CK_SecondCount >= SW_CK_SECOND_COUNT_PERIOD)
		{
			SW_CK_SecondCount = 0;

		}


	}
}

bool SW_CK_SendLongCountSerial()
{
	Serial.print("#CL,");
	Serial.print(SW_CK_CKLongCount);
	Serial.println(";");
	return true;

}


bool SW_CK_InterruptOccurred()
{
	return SW_CK_CKInterrupted;
}

int SW_CK_GetCKShortCount()
{
	return SW_CK_CKShortCountTo9;
}

int SW_CK_GetCKMedCount()
{
	return SW_CK_CKMedCount;
}

int SW_CK_GetCKLongCount()
{
	return SW_CK_CKLongCount;
}

int SW_CK_GetCK90sCount()
{
	return SW_CK_CK90sCount;
}

int SW_CK_GetSecondCount()
{
	return SW_CK_SecondCount;
}

int SW_CK_GetSubSecondCount()
{
	return SW_CK_CKShortCountTo4;
}

// This will be true once every 5th second
bool SW_CK_EveryFifthSecond()
{
	if(SW_CK_EveryFifthSecondNotCalled)
	{

	SW_CK_EveryFifthSecondNotCalled = false;
	return true;
	}
	else
	{
		return false;
	}

}

// This will be true once every second
bool SW_CK_EverySecond()
{
	if(SW_CK_EverySecondNotCalled)
		{
	SW_CK_EveryFifthSecondNotCalled = false;
		return true;
		}
		else
		{
		return false;
		}


}

