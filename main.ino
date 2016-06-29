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
#include "LowPower.h"

#include "I2Cdev.h"
#include "HTU21D.h"


unsigned short int curSleepCycle;
unsigned short int curLogCycle;
unsigned short int curNetCycle;

unsigned short int maxSleepCycle;
unsigned short int maxLogCycle;
unsigned short int maxNetCycle;

unsigned short int curMinLogCycle;
unsigned short int maxMinLogCycle;
unsigned short int lastMinLogCycle;

HTU21D dht;

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

	maxLogCycle = CYCLES_LOG_COUNT;
	maxNetCycle = CYCLES_NET_COUNT;
	maxSleepCycle = maxLogCycle;	//CYCLES_SLEEP_COUNT;

	maxMinLogCycle = DAILY_LOG_PERIOD;
      
	setupPeripheralsControl();
	dht.initialize();
	


	powerPeripherals( (unsigned char)0, 0 );	/* disable all peripherals */
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

void mySleep()
{
//	D("sleep... ");
	Serial.flush();
	LowPower.powerDown(LP_SLEEP_MODE, ADC_OFF, BOD_OFF);
//	Serial.begin( 9600 );
//	Dln("wakeup!");
}



void loop()
{
  datetime_t dt;
  float f1, f2;
  unsigned long f3a, f3b;



    powerPeripherals(1,1);
    getTime(&dt);
    powerPeripherals(0,0);
    
    lastMinLogCycle = getMinutes(&dt);
    curMinLogCycle = lastMinLogCycle;
  

  
  	while(1) {
  		mySleep();
	
  		curSleepCycle++;
  		D("curSleepCycle ");Dln(curSleepCycle);

		if(curSleepCycle >= maxSleepCycle) {
		    /* do not reset curSleepCycle yet, since PERIOD might not
		     * have been reached yet, do it inside the inner loop */
	
		    D("curSleepCycle >= maxSleepCycle ");Dln(curSleepCycle);
			f3a = readVcc();
			powerPeripherals(1, 1);
			getTime(&dt);
            powerPeripherals(0, 0);

			curMinLogCycle = getMinutes(&dt);
			if(curMinLogCycle - lastMinLogCycle >= maxMinLogCycle) {

			    D("curMinLogCycle - lastMinLogCycle >= MaxMinLogCycle ");
			    D(curMinLogCycle); D(" "); Dln(lastMinLogCycle);
			    lastMinLogCycle = curMinLogCycle;

			    curSleepCycle = 0;	/* reset sleep cycle */
			  
			    powerPeripherals(1,1);
    			f1 = dht.getTemperature();
    			f2 = dht.getHumidity();
    			f3b = readVcc();
  	    		powerPeripherals(0, 0);
			
		    	/* date */
		    	convertDate2Str(tmp, &dt);
		    	Serial.print(tmp); Serial.print(",");
			
		    	/* time */
		    	convertTime2Str(tmp, &dt);
		    	Serial.print(tmp); Serial.print(",");
			
		    	/* power voltage */
		    	Serial.print(f3a); Serial.print(",");
			
		    	Serial.print(f3b); Serial.print(",");
			
		    	/* temperature */
		    	Serial.print(f1); Serial.print(",");
			
		    	Serial.print(f2); Serial.print("\n");

		    	Serial.flush();
            }
		}
	}
}



/* main loop of the project */
void loop1()
{
  char doLog=0, doNet=0;



  	/* prepare to sleep and sleep */
  	D("Current cycle : (log) "); D(doLog); D("  (net) "); Dln(doNet);
  	D("About to sleep... ");
  	LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  	
  	
  	
  	/* wake up! */
  	curSleepCycle++;
  	if(curSleepCycle >= maxSleepCycle) {
  		/* we should check real-time clock if we are ready to log or do net communication */
  		
  		powerRTC( 1, 2 );	/* powerup RTC module, and delay for 2 msec to stabilize */

        }


  		
  
  if(curLogCycle >= maxLogCycle) {
    doLog=1;
    curLogCycle = 0;
  }
  
  if(curNetCycle >= maxNetCycle) {
    doNet=1;
    curNetCycle = 0;
  }    

  D("Current cycle : (log) "); D(doLog); D("  (net) "); Dln(doNet);
  
  doActions(doLog, doNet);

  D("About to sleep... ");
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);

  /* so! we woke up again */
  Dln("wake up!");
  curLogCycle++;
  curNetCycle++;
}
