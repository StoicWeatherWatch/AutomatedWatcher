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

// For some reason this cannot go in the .h file
bool SW_CK_CKInterrupted = false;

// An interrupt is received at a pin every 0.25 s or 4 Hz

// Short is every 9 or 2.25 s
int SW_CK_CKShortCount = 0;

// Counts every 9th short to count in increments of 2.25 s
// Reset every 4 or every 9 seconds
int SW_CK_CKMedCount = 0;

// Counts every 9th short to count in increments of 2.25 s
// Reset every 40 or every 90 seconds or 360 interrupts
int SW_CK_CKLongCount = 0;

// Counts every 90s interval. 2 for 3 min, 40 for 1 hour. 960 for 1 day
int SW_CK_CK90sCount = 0;



bool SW_CK_ClockSetup()
{
	attachInterrupt(digitalPinToInterrupt(SW_CK_CLOCKPIN), SW_CK_ClockInterrupt,RISING);

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

	SW_CK_CKShortCount++;
	if(SW_CK_CKShortCount >= SW_CK_SHORT_COUNT_PERIOD)
	{
		SW_CK_CKShortCount = 0;

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
	return SW_CK_CKShortCount;
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

