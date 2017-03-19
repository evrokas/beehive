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
#include <Wire.h>

#include "bms.h"
#include "utils.h"
#include "rtc.h"
#include "accel.h"
#include "thermal.h"
#include "LowPower.h"
#include "gsm.h"

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

/*
 * Noticed that some times reading the ADXL345 without first enabling
 * peripherals, returns trash from the ADXL345 IC.
 * Probably, the RTC I am using has internal pullups which are used also for
 * accessing ADXL345 under normal conditions.  One solution found was to
 * initialize Wire interface (I2C) and set minimum speed (must read the
 * manual), then we just expect, the board to work without pull-up
 * resistors.
 *
 */
 
	curLogCycle = 0;
	curNetCycle = 0;
	curSleepCycle = 0;


	maxLogCycle = CYCLES_LOG_COUNT;
	maxMinLogCycle = DAILY_LOG_PERIOD;

	maxNetCycle = CYCLES_NET_COUNT;
	maxMinNetCycle = DAILY_NET_PERIOD;

#if 0
	/* hardwire log cycles for debugging purposes */
	maxMinLogCycle = 2;
	maxMinNetCycle = 4;
#endif


	maxSleepCycle = maxLogCycle;
      
	setupPeripheralsControl();

//	TWBR = 152;		/* switch to 25KHz I2C interface */

	powerRTC( 1, 10 );
	powerPeripherals(1,1);
	powerGPRSGPS( 0 );

	therm_init();
	rtc_init();
	accel_init();

	powerPeripherals(0,0);	/* disable all peripherals */
	powerRTC( 0, 1 );
	Serial.begin( 9600 );
	Dln("\n\n\nHello world. Sleep cycling begins!");
	sprintf(tmp, "Sleep duration %d sec, cycles %d", SLEEP_CYCLE, maxSleepCycle);
    Dln(tmp);

    sprintf(tmp, "Log period %d min, Net period %d min", DAILY_LOG_PERIOD, DAILY_NET_PERIOD);
    Dln(tmp);

    sprintf(tmp, "Log freq %d, net freq %d", maxMinLogCycle, maxMinNetCycle);
    Dln(tmp);


#if	HAVE_GSM_GPRS == 1
	gsm_init();
//	powerGPRSGPS( 0 );
#endif


#if 0
	/* run this code only to setup date and time */
	powerPeripherals( 1, 1 );
	setTime(0,45, 16, 4, 1, 3, 17 );
	powerPeripherals( 0, 0 );

	/* stop program execution */
	Serial.println("Now you must comment out RTC writing code, and reload program to board.\n");
	while(true);
#endif
}


void mySleep()
{
	Serial.flush();
	LowPower.powerDown(LP_SLEEP_MODE, ADC_OFF, BOD_OFF);
	Wire.begin();
//	TWBR = 152;		/* switch to 25KHz I2C interface */

#if 0
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0); // set DS3231 register pointer to 00h
	Wire.endTransmission();
#endif
}


char tmpb[32];
int16_t ax, ay, az;
int poweredGSM=0;


void loop()
{
  datetime_t dt;
  float f1, f2;
  unsigned long f3a, f3b;

#define DEBUG_SLEEP_CYCLE	1

	/* get initial time */
//    powerPeripherals(1,1);
		powerRTC( 1, 10 );
    rtc_getTime(&dt);
    powerRTC( 0, 1 );
//    powerPeripherals(0,0);
    
    lastMinLogCycle = lastMinNetCycle = rtc_getMinutes(&dt);
    curMinLogCycle = lastMinLogCycle;
    curMinNetCycle = lastMinNetCycle;

#ifdef DEBUG_SLEEP_CYCLE
	D("INIT: curMinLogCycle - lastMinLogCycle >= MaxMinLogCycle ");
	D(curMinLogCycle); D(" "); Dln(lastMinLogCycle);
#endif



	/* enter endless loop */
  	while(1) {
  		mySleep();
	
  		curSleepCycle++;

		
#ifdef DEBUG_SLEEP_CYCLE
  		D("curSleepCycle ");Dln(curSleepCycle);
#endif

		if(curSleepCycle >= maxSleepCycle) {
		    /* do not reset curSleepCycle yet, since PERIOD might not
		     * have been reached yet, do it inside the inner loop */
	
#ifdef DEBUG_SLEEP_CYCLE
			D(F("curSleepCycle >= maxSleepCycle "));Dln(curSleepCycle);
#endif			

			f3a = readVcc();
			
			powerRTC( 1, 10 );
//			powerPeripherals(1, 1);

			rtc_getTime(&dt);
			powerRTC( 0, 1 );
			
			/* do not power off peripherals, since they might be needed, by next check */
//          powerPeripherals(0, 0);

			curMinLogCycle = curMinNetCycle = rtc_getMinutes(&dt);
			if(curMinLogCycle - lastMinLogCycle >= maxMinLogCycle) {

#ifdef DEBUG_SLEEP_CYCLE
			    D( F("curMinLogCycle - lastMinLogCycle >= MaxMinLogCycle ") );
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
//		    	convertDate2Str(tmp, &dt);
//		    	Serial.print(tmp); Serial.print(",");
			
		    	/* time */
//		    	convertTime2Str(tmp, &dt);
//		    	Serial.print(tmp); Serial.print(",");
			
		    	/* power voltage, before powering peripherals */
		    	Serial.print(f3a); Serial.print(",");
			
				/* power voltage, after powering peripherals */
		    	Serial.print(f3b); Serial.print(",");
			
		    	/* temperature */
		    	Serial.print(f1); Serial.print(",");
			
		    	Serial.print(f2); Serial.print("\n\r");

		    	Serial.flush();
			}
  
//			powerPeripherals( 0,0 );
            
			if(curMinNetCycle - lastMinNetCycle >= maxMinNetCycle) {
#ifdef DEBUG_SLEEP_CYCLE
			    D("curMinNetCycle - lastMinNetCycle >= MaxMinNetCycle ");
			    D(curMinNetCycle); D(" "); Dln(lastMinNetCycle);
#endif

				if( poweredGSM > 5 ) {
					Dln("GSM was not ready after 5 tries. Skip session");
					
					/* module is not ready after 5 cycles, so there might be a problem,
					 * skip network communication this time */
					poweredGSM = 0;
					powerGPRSGPS( 0 );

					lastMinNetCycle = curMinNetCycle;
					curSleepCycle = 0;	/* reset sleep cycle */

					continue;
				}
				
				if(!poweredGSM) {
					poweredGSM++;
					D("GSM is not ready on try ... "); Dln( poweredGSM );
					
					/* only apply power on the first time */
					if(poweredGSM == 1)
						powerGPRSGPS( 1 );
					memset(tmpb, 0, sizeof( tmpb ));
					continue;
				}

				poweredGSM++;
				if( !gsm_moduleInfo() ) {
					D("GSM module is not ready yet! try ... "); Dln( poweredGSM );
					/* module is not yet ready, skip this cycle */
					continue;
				}
				
				Dln("GSM module is ready!");
				/* so module is ready */

				Dln("Doing network stuff");

#if 0
			    gsm_activateBearerProfile("internet.cyta.gr", "", "");
			    http_getRequest("https://52.7.7.190", "/update?api_key=7EE6FEDU182QNN2U&1=60&2=100" );
			    gsm_deactivateBearerProfile();
#endif

//				gsm_getBattery();

//				Dln("Setting pin");
//				gsm_sendPin( "1234" );

				delay( 1000 );

#define CF( str )	(char *)( str )
				gsm_sendcmd(CF( F( "AT+CFUNC?\n\r" ) ) );
				
				gsm_relayOutput( Serial );
								
				powerGPRSGPS( 0 );
				poweredGSM = 0;
				
				lastMinNetCycle = curMinNetCycle;
				curSleepCycle = 0;	/* reset sleep cycle */

//				gsm_sendrecvcmd("ATI\n", tmpb);
//				Dln(tmpb);
			    
			    /* 
			     * put
			     * code
			     * for 
			     * GSM/GPRS 
			     * network 
			     * connection
			     */
			    

			}	/* in net cycle  */
		
		}	/* Sleep Cycle */
	
	}	/* main while loop  */
}
