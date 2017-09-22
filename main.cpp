/*
 * main.cpp - main file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-16-17
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
#include "data.h"


/* counter of sleep cycles */
uint32_t cntSleepCycle;

uint16_t curSleepCycle;
uint16_t curLogCycle;
uint16_t curNetCycle;

uint16_t maxSleepCycle;
uint16_t maxLogCycle;
uint16_t maxNetCycle;

uint16_t curMinLogCycle;
uint16_t maxMinLogCycle;
uint16_t lastMinLogCycle;

uint16_t curMinNetCycle;
uint16_t maxMinNetCycle;
uint16_t lastMinNetCycle;


char tmp[56], tmp2[16];

#ifndef NODE_ID

#define NODE_ID	1088

#endif



/* setup steps of the project */
void setup()
{
	Dinit;
	Serial.begin( 9600 );

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


	cntSleepCycle = 0;

	maxLogCycle = CYCLES_LOG_COUNT;
	maxMinLogCycle = DAILY_LOG_PERIOD;

	maxNetCycle = CYCLES_NET_COUNT;
	maxMinNetCycle = DAILY_NET_PERIOD;

#if 0
	/* hardwire log cycles for debugging purposes */
	maxMinLogCycle = 2;
	maxMinNetCycle = 4;
#endif
	Dln("hello world!");

	maxSleepCycle = maxLogCycle;
      
	setupPeripheralsControl();

	//Wire.init();
//	TWBR = 152;		/* switch to 25KHz I2C interface */

	powerRTC( 1, 10 );
	powerPeripherals(1,1);
	powerGPRSGPS( 0 );

	therm_init();
	rtc_init();
	//accel_init();

	powerPeripherals(0,0);	/* disable all peripherals */
	powerRTC( 0, 1 );
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
	
	/* increase sleep cycle counter */
	cntSleepCycle++;
	
	
//	TWBR = 152;		/* switch to 25KHz I2C interface */

#if 0
	Wire.beginTransmission(RTC_I2C_ADDRESS);
	Wire.write(0); // set DS3231 register pointer to 00h
	Wire.endTransmission();
#endif
}


//char tmpb[100];
int16_t ax, ay, az;
int poweredGSM=0;


void loop()
{
  datetime_t dt;
//  float f1, f2;
//  unsigned long f3a, f3b;
  datablock_t	db;


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
  		D("[");D(cntSleepCycle);D("] ");
  		
  		D("curSleepCycle ");Dln(curSleepCycle);
#endif

		if(curSleepCycle >= maxSleepCycle) {
		    /* do not reset curSleepCycle yet, since PERIOD might not
		     * have been reached yet, do it inside the inner loop */
	
#ifdef DEBUG_SLEEP_CYCLE
			D(F("curSleepCycle >= maxSleepCycle "));Dln(curSleepCycle);
#endif			
			db.nodeId = NODE_ID;
			db.batVolt = readVcc();
			
			powerRTC( 1, 10 );
			displayTime();
			rtc_getTime(&dt);
			powerRTC( 0, 1 );
			
			curMinLogCycle = curMinNetCycle = rtc_getMinutes(&dt);
			if(abs(curMinLogCycle - lastMinLogCycle) >= maxMinLogCycle) {
					/* maxMinLogCycle minutes have passed since last cycle
					 * do log-ging of data */

#ifdef DEBUG_SLEEP_CYCLE
			    D( ("curMinLogCycle - lastMinLogCycle >= MaxMinLogCycle ") );
			    D(curMinLogCycle); D(" "); Dln(lastMinLogCycle);
#endif

			    lastMinLogCycle = curMinLogCycle;

			    curSleepCycle = 0;	/* reset sleep cycle */

			    powerRTC(1, 10);
			    powerPeripherals(1,50);
			    db.bhvTemp = therm_getTemperature() * 100;
			    db.bhvHumid = therm_getHumidity() * 100;
			    
			    powerPeripherals(0,0);
			    powerRTC(0, 0);
			    
					db.rtcDateTime = unixtime( &dt );
					
					Dln("after powering off peripherals");

#define DEB_STUFF

#ifdef DEB_STUFF
          Serial.print(">>,");

		    	/* power voltage, before powering peripherals */
		    	Serial.print(db.batVolt); Serial.print(",");
			
		    	/* temperature */
		    	Serial.print(db.bhvTemp); Serial.print(",");
			
					/* humidity */
		    	Serial.print(db.bhvHumid); Serial.print("\n\r");

		    	Serial.flush();
#endif

/*

					if( mem_pushDatablock( &db ) ) {
						Serial.println("pushed datablock to EEPROM successfully.");
					} else {
						Serial.println("could not push datablock to EEPROM.");
					}
*/
					
					/* put here code to store in EEPROM memory data blocks until
					 * net cycle is reached, and data are forwarded to internet server */


			}
  
			if(abs(curMinNetCycle - lastMinNetCycle) >= maxMinNetCycle) {
				/* maxMinNetCycle minutes have passed since last cycle
				 * do net-working stuff */

#ifdef DEBUG_SLEEP_CYCLE
			    D("curMinNetCycle - lastMinNetCycle >= MaxMinNetCycle ");
			    D(curMinNetCycle); D(" "); Dln(lastMinNetCycle);
#endif

#if 0
				if( poweredGSM >= 4 ) {
					Dln("GSM was not ready after 3 tries. Skip NETWORK session");
					
					/* module is not ready after 3 cycles, so there might be a problem,
					 * skip network communication this time */
					poweredGSM = 0;
					powerGPRSGPS( 0 );

					lastMinNetCycle = curMinNetCycle;
					curSleepCycle = 0;	/* reset sleep cycle */

					continue;
				}
#endif
				
				if(!poweredGSM) {
					poweredGSM++;
					D("Trying to power-up GSM/GPRS module - try "); Dln( poweredGSM );
					
					/* only apply power on the first time */
					if(poweredGSM == 1)
						powerGPRSGPS( 1 );

				}

				//poweredGSM++;
				
				while( poweredGSM < 10 ) {
					if( !gsm_moduleInfo() ) {
						D( F("GSM module is not ready yet! try ... ") ); Dln( poweredGSM );
						/* module is not yet ready, skip this cycle */

						delay(5000);
					} else
						break;	/* module ready! */
					
					poweredGSM++;
				}

				Dln( poweredGSM );
				
				if(poweredGSM == 10) {
					// module was not ready, so skip this session
					poweredGSM = 0;
					powerGPRSGPS( 0 );

					lastMinNetCycle = curMinNetCycle;
					curSleepCycle = 0;	/* reset sleep cycle */

					continue;
				}
				
#define CF( str )	(char *)( str )
				
				Dln( F("GSM module is ready!") );
				/* so module is ready */

				Dln( F("Doing network stuff") );

				if ( gsm_sendPin( CF( "1234" ) ) ) {
					Dln( F("PIN set OK") );
				} else {
					Dln( F("PIN was NOT set") );
				}

				if(!gsm_sendrecvcmdtimeout( CF(""), CF("SMS Ready\r\n"), 10)) {
//					Dln("module out of sync....");
				};
//				 else Dln("module in sync....");
				

				/* disable local echo */
				gsm_sendcmd(CF("ATE0\r\n"));
				
		
				{
				uint8_t r;
				do {
					//uint8_t	r;
						gsm_getRegistration( r );
						if((r == 1) || (r == 5))break;
				} while( 1 );		/* set a timeout here */
				}
				
				
				do {
					uint16_t ii;
					uint8_t iii;
					
					if( gsm_getBattery( ii ) ) {
//						Serial.print("Battery level: " ); Serial.println( ii );
						db.gsmVolt = ii;
					}

					if( gsm_getSignalQuality( iii ) ) {
//						Serial.print("Signal quality: " ); Serial.println( iii );
						db.gsmSig = iii;
					}
				} while(0);
				
				if(gsm_activateBearerProfile("internet.cyta.gr", "", "")) {
					if( http_initiateGetRequest() ) {
						if( http_send_datablock( db ) ) {
						} else Serial.println(F("error: could not send data block"));
					} else Serial.println(F("error: could not initiate get request"));
				} else Serial.println(F("error: could not activate bearer profile"));
				
				http_terminateRequest();						
				gsm_deactivateBearerProfile();

//				delay(5000);

				/* put here code to read data blocks from EEPROM,
				 * and forward them to the internet server.
				 * there are 2 options, (1) a series of GET requests to the server
				 * and (2) one POST request with all data included in the payload
				 */
				 

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

