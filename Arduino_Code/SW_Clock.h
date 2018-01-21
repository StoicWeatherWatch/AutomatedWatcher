/*
 * SW_Clock.h
 *
 *  Created on: 2018-01-18
 *      Author: StoicWeather
 */



#if(ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#ifndef SW_CLOCK_H_
#define SW_CLOCK_H_

#define SW_CK_CLOCKPIN 2

//A 4 Hz clock is assumed
#define SW_CK_SHORT_COUNT_PERIOD 9
#define SW_CK_MED_COUNT_PERIOD 4
#define SW_CK_LONG_COUNT_PERIOD 40


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


#endif /* SW_CLOCK_H_ */
