/*
 * SW_Clock.h
 *
 *  Created on: 2018-02-06
 *      Author: StoicWeather
 */

// This code attaches an interrupt to a pin and expects a 4 Hz square wave. (A clean 4 Hz square wave)



#if(ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif


#ifndef SW_CLOCK_H_
#define SW_CLOCK_H_

#define SW_CLOCK_INTERNAL_TIMER_ACTIVE

#include "SW_Helper_Functions.h"

// Set to report total uptime every 90 seconds
#define SW_CK_REPORT_UP_TIME
//Set to report in *AUT,<Uptime / 90 seconds>; format
//#define SW_CK_REPORT_UP_TIME_MACHINE_READABLE


//  Clock signal input pin
#define SW_CK_CLOCK_PIN 2

//A 4 Hz clock is assumed
#define SW_CK_SHORT_COUNT_PERIOD 9
#define SW_CK_MED_COUNT_PERIOD 4
#define SW_CK_LONG_COUNT_PERIOD 40
#define SW_CK_SECOND_COUNT_PERIOD 30


// In SW_Clock.cpp
//bool SW_CK_CKInterrupted = false;


bool SW_CK_ClockSetup();

void SW_CK_ClockInterrupt();
void SW_CK_ClockIntruptProcessing();

bool SW_CK_InterruptOccurred();

int SW_CK_GetCKShortCount();
int SW_CK_GetCKMedCount();
int SW_CK_GetCKLongCount();
int SW_CK_GetCK90sCount();
int SW_CK_GetSecondCount();
int SW_CK_GetSubSecondCount();

bool SW_CK_EveryFifthSecond();
bool SW_CK_EverySecond();

bool SW_CK_SendLongCountSerial();

#ifdef SW_CLOCK_INTERNAL_TIMER_ACTIVE

bool SW_Ck_Internal_Timer_Check_Time();
void SW_Ck_Internal_Timer_Initialize();

#endif /*SW_CLOCK_INTERNAL_TIMER_ACTIVE*/


#endif /* SW_CLOCK_H_ */
