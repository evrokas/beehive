/*
 * main.ino - main file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-16
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#include <Arduino.h>

#include "bms.h"
#include "utils.h"
#include "rtc.h"
#include "accel.h"
#include "thermal.h"
#include "LowPower.h"

unsigned short int curSleepCycle;
unsigned short int curLogCycle;
unsigned short int curNetCycle;

unsigned short int maxSleepCycle;
unsigned short int maxLogCycle;
unsigned short int maxNetCycle;

unsigned short int curMinLogCycle;
unsigned short int maxMinLogCycle;
unsigned short int lastMinLogCycle;

unsigned short int curMinNetCycle;
unsigned short int maxMinNetCycle;
unsigned short int lastMinNetCycle;


char tmp[56], tmp2[16];


/* sleep mcu for 8 seconds */
void bmsSleep()
{
  /* do sleep */
}
  

/* setup steps of the project */
void setup()
{
	Dinit;

  
	curLogCycle = 0;
	curNetCycle = 0;
	curSleepCycle = 0;

	maxSleepCycle = maxLogCycle;

	maxLogCycle = CYCLES_LOG_COUNT;
	maxMinLogCycle = DAILY_LOG_PERIOD;

	maxNetCycle = CYCLES_NET_COUNT;
	maxMinNetCycle = DAILY_NET_PERIOD;

      
	setupPeripheralsControl();
	therm_init();
	rtc_init();


	powerPeripherals(0,0);	/* disable all peripherals */
	Serial.begin( 9600 );
	Dln("\n\n\nHello world. Sleep cycling begins!");
	sprintf(tmp, "Sleep duration %d sec, cycles %d", SLEEP_CYCLE, maxSleepCycle);
    Dln(tmp);

    sprintf(tmp, "Log period %d min, Net period %d min", DAILY_LOG_PERIOD, DAILY_NET_PERIOD);
    Dln(tmp);

#if 0
	/* run this code only to setup date and time */
	powerPeripherals( 1, 1 );
	setTime(0, 29, 2, 0, 21, 6, 16);
	powerPeripherals( 0, 0 );
#endif
}


void mySleep()
{
	Serial.flush();
	LowPower.powerDown(LP_SLEEP_MODE, ADC_OFF, BOD_OFF);
}



void loop()
{
  datetime_t dt;
  float f1, f2;
  unsigned long f3a, f3b;


	/* get initial time */
    powerPeripherals(1,1);
    rtc_getTime(&dt);
    powerPeripherals(0,0);
    
    lastMinLogCycle = lastMinNetCycle = rtc_getMinutes(&dt);
    curMinLogCycle = lastMinLogCycle;
    curMinNetCycle = lastMinNetCycle;

	/* enter endless loop */
  	while(1) {
  		mySleep();
	
  		curSleepCycle++;

#define DEBUG_SLEEP_CYCLE	1
		
#ifdef DEBUG_SLEEP_CYCLE
  		D("curSleepCycle ");Dln(curSleepCycle);
#endif

		if(curSleepCycle >= maxSleepCycle) {
		    /* do not reset curSleepCycle yet, since PERIOD might not
		     * have been reached yet, do it inside the inner loop */
	
#ifdef DEBUG_SLEEP_CYCLE
			D("curSleepCycle >= maxSleepCycle ");Dln(curSleepCycle);
#endif			

			f3a = readVcc();
			powerPeripherals(1, 1);
			rtc_getTime(&dt);
            powerPeripherals(0, 0);

			curMinLogCycle = curMinNetCycle = rtc_getMinutes(&dt);
			if(curMinLogCycle - lastMinLogCycle >= maxMinLogCycle) {

#ifdef DEBUG_SLEEP_CYCLE
			    D("curMinLogCycle - lastMinLogCycle >= MaxMinLogCycle ");
			    D(curMinLogCycle); D(" "); Dln(lastMinLogCycle);
#endif

			    lastMinLogCycle = curMinLogCycle;

			    curSleepCycle = 0;	/* reset sleep cycle */
			  
			    powerPeripherals(1,1);
    			f1 = therm_getTemperature();
    			f2 = therm_getHumidity();
    			f3b = readVcc();
  	    		powerPeripherals(0, 0);
			
                Serial.print(">>,");
		    	/* date */
		    	convertDate2Str(tmp, &dt);
		    	Serial.print(tmp); Serial.print(",");
			
		    	/* time */
		    	convertTime2Str(tmp, &dt);
		    	Serial.print(tmp); Serial.print(",");
			
		    	/* power voltage, before powering peripherals */
		    	Serial.print(f3a); Serial.print(",");
			
				/* power voltage, after powering peripherals */
		    	Serial.print(f3b); Serial.print(",");
			
		    	/* temperature */
		    	Serial.print(f1); Serial.print(",");
			
		    	Serial.print(f2); Serial.print("\n");

		    	Serial.flush();
            }
            
			if(curMinNetCycle - lastMinNetCycle >= maxMinNetCycle) {
#ifdef DEBUG_SLEEP_CYCLE
			    D("curMinNetCycle - lastMinNetCycle >= MaxMinNetCycle ");
			    D(curMinNetCycle); D(" "); Dln(lastMinNetCycle);
#endif

			    lastMinNetCycle = curMinNetCycle;

			    curSleepCycle = 0;	/* reset sleep cycle */
			    
			    /* 
			     * put
			     * code
			     * for 
			     * GSM/GPRS 
			     * network 
			     * connection
			     */
			    

			}
		}
	}
}

void doActions(char doLog, char doNet)
{
	/* do nothing if not log and not net */
	if(!doLog && !doNet)return;

	/* check if we should power peripherals on */
	if(doLog || doNet)
		powerPeripherals( (unsigned char)1, 2 );

	if(doLog) {
		/* so read the sensors and log data */
	}
  
	if(doNet) {
		/* we must setup network communication and send data to the server */
	}
  

	/* shutdown peripherals */
	if(doLog || doNet)
		powerPeripherals( (unsigned char)0, 2 );
}
