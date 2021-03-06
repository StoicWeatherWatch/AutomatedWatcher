/*
 * SW_Clock.cpp
 *
 *  Created on: 2018-03-23
 *      Author: StoicWeather
 */

#include "SW_Clock.h"

#include <Arduino.h>


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

#ifdef SW_CLOCK_INTERNAL_TIMER_ACTIVE

unsigned long SW_Ck_Internal_Timer_timeLast = 0;
unsigned long SW_Ck_Internal_Timer_tineNow = 0;


#endif /*SW_CLOCK_INTERNAL_TIMER_ACTIVE*/

#ifdef SW_CLOCK_REPORT_MILLS

unsigned long SW_Ck_Mills_Counter_timeLast = 0;
unsigned long SW_Ck_Mills_Counter_timetotal = 0;
unsigned long SW_Ck_Mills_Counter_timeLastSec = 0;

#endif /*SW_CLOCK_REPORT_MILLS*/




bool SW_CK_ClockSetup()
{
	attachInterrupt(digitalPinToInterrupt(SW_CK_CLOCK_PIN), SW_CK_ClockInterrupt,RISING);

	Serial.println(F("#Clock Interrupt Starting;"));

#ifndef SW_CLOCK_INTERNAL_TIMER_ACTIVE
	while(!SW_CK_CKInterrupted)
	{
		// Wait for clock. Need to have something here. Empty did not work.
		delay(100);
	}
#endif /*SW_CLOCK_INTERNAL_TIMER_ACTIVE*/
#ifdef SW_CLOCK_INTERNAL_TIMER_ACTIVE

	Serial.println(F("# USING INTERNAL TIMER!!!!!;"));

SW_Ck_Internal_Timer_Initialize();

#endif /*SW_CLOCK_INTERNAL_TIMER_ACTIVE*/

#ifdef SW_CLOCK_REPORT_MILLS

SW_Ck_Mills_Counter_timeLast = millis();
SW_Ck_Mills_Counter_timetotal = millis();

#endif /*SW_CLOCK_REPORT_MILLS*/

	Serial.println(F("#Clock Interrupt Functional;"));
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

#ifdef SW_CLOCK_REPORT_MILLS

SW_Ck_Mills_Counter_timetotal = millis();

Serial.print(F("#ms ct "));
Serial.print(SW_Ck_Mills_Counter_timetotal - SW_Ck_Mills_Counter_timeLast);
Serial.println(F(" ;"));

SW_Ck_Mills_Counter_timeLast = SW_Ck_Mills_Counter_timetotal;

#endif /*SW_CLOCK_REPORT_MILLS*/
#ifdef REPORT_MEMORY_EVERY_TICK
extern int __heap_start, *__brkval;
	int v;
	Serial.print(F("# Free RAM  "));
	Serial.print((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
	Serial.println(F(";"));
	// END TEST
#endif /*REPORT_MEMORY_EVERY_TICK*/


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
			Serial.print(F("#Uptime, "));
			int Tminutes = (SW_CK_CK90sCount * 3) / 2;
			Serial.print(Tminutes);
			if (((SW_CK_CK90sCount * 3) % 2) == 1)
			{
				Serial.print(F(".5"));
			}
			Serial.println(F(" minutes;"));


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
			SW_CK_EveryFifthSecondNotCalled = true;

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

#ifdef SW_CLOCK_INTERNAL_TIMER_ACTIVE
	return SW_Ck_Internal_Timer_Check_Time();
#endif /*SW_CLOCK_INTERNAL_TIMER_ACTIVE*/

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

#ifdef SW_CLOCK_REPORT_MILLS

SW_Ck_Mills_Counter_timetotal = millis();

Serial.print(F("#s ct "));
Serial.print(SW_Ck_Mills_Counter_timetotal - SW_Ck_Mills_Counter_timeLastSec);
Serial.println(F(" ms;"));

SW_Ck_Mills_Counter_timeLastSec = SW_Ck_Mills_Counter_timetotal;

#endif /*SW_CLOCK_REPORT_MILLS*/


		SW_CK_EverySecondNotCalled = false;
		return true;
	}
	else
	{
		return false;
	}


}

#ifdef SW_CLOCK_INTERNAL_TIMER_ACTIVE

bool SW_Ck_Internal_Timer_Check_Time()
{
	SW_Ck_Internal_Timer_tineNow = millis();

	if((SW_Ck_Internal_Timer_tineNow - SW_Ck_Internal_Timer_timeLast) >= SW_CLOCK_INTERNAL_TIMER_PERIOD)
	{
		SW_Ck_Internal_Timer_timeLast = millis();
		return true;
	}

	return false;

}

void SW_Ck_Internal_Timer_Initialize()
{
	SW_Ck_Internal_Timer_timeLast = millis();
}

#endif /*SW_CLOCK_INTERNAL_TIMER_ACTIVE*/

