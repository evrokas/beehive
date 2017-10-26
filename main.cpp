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
#include <EEPROM.h>

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


#ifndef NODE_ID

#define NODE_ID	1088

#endif

uint16_t	addrNodeId;
uint16_t	addrAPIKEY;
uint16_t	addrAPN;
uint16_t	addrUSER;
uint16_t	addrPASS;
uint16_t	addrSERVER;
uint16_t	addrPORT;


void initalizeEEPROM()
{
	addrNodeId	= eepromGetAddr( 2 );
	addrAPIKEY	= eepromGetAddr( 8 );
	addrAPN			= eepromGetAddr( 32 );
	addrUSER		= eepromGetAddr( 16 );
	addrPASS		= eepromGetAddr( 16 );
	addrSERVER	= eepromGetAddr( 4 );			/* 255.255.255.255, each byte is stored separately */
	addrPORT		= eepromGetAddr( 2 );			/* 0-65535, 2 bytes long */
}


void initializeCounters()
{
 	/* current logging cycle counter */
	curLogCycle = 0;
	
	/* current networking cycle counter */
	curNetCycle = 0;
	
	/* current sleep cycle counter */
	curSleepCycle = 0;

	/* global sleep cycle counter */
	cntSleepCycle = 0;

 	maxMinLogCycle = DAILY_LOG_PERIOD;
	maxMinNetCycle = DAILY_NET_PERIOD;

#if 0
	/* hardwire log cycles for debugging purposes */
	maxMinLogCycle = 2;
	maxMinNetCycle = 4;
#endif

	maxSleepCycle = CYCLES_SLEEP_COUNT;
}


/* setup steps of the project */
void setup()
{
	delay(2000);
	
	Dinit;
	Serial.begin( 9600 );

	Serial.println(F("Beehive Monitoring System (c) 2015,16,17. All Rights reserved"));
	Serial.println(F("(c) Evangelos Rokas <evrokas@gmail.com>"));
	Serial.print(F("System Board version ")); Serial.print( BOARD_REVISION );
	Serial.print(F("  Firmware version ")); Serial.println( FIRMWARE_REVISION );
	
	delay(2000);

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

	initializeCounters(); 
      
	setupPeripheralsControl();


//Wire.init();
//	TWBR = 152;		/* switch to 25KHz I2C interface */

	powerRTC( 1, 10 );
	powerPeripherals(1,1);
	powerGPRSGPS( 0 );

	/* always have RTC(!) */
	rtc_init();

#if HAVE_THSENSOR == 1
	therm_init();
#endif

#if HAVE_ACCEL == 1
	accel_init();
#endif

#if	HAVE_GSM_GPRS == 1
		gsm_init();
#endif


	powerPeripherals(0,0);	/* disable all peripherals */
	powerRTC( 0, 1 );


//	Dln("\n\n\nHello world. Sleep cycling begins!");
//	sprintf(tmp, "Sleep duration %d sec, cycles %d", SLEEP_CYCLE, maxSleepCycle);
//  Dln(tmp);

//  sprintf(tmp, "Log period %d min, Net period %d min", DAILY_LOG_PERIOD, DAILY_NET_PERIOD);
//  Dln(tmp);

//  sprintf(tmp, "Log freq %d, net freq %d", maxMinLogCycle, maxMinNetCycle);
//  Dln(tmp);

}


void mySleep()
{
	Serial.flush();
	LowPower.powerDown(LP_SLEEP_MODE, ADC_OFF, BOD_OFF);
	Wire.begin();
	
	/* increase sleep cycle counter */
	cntSleepCycle++;
	
//	TWBR = 152;		/* switch to 25KHz I2C interface */
}


//char tmpb[100];
int16_t ax, ay, az;
int poweredGSM=0;
datetime_t dt;


void setupLoop()
{
	powerRTC( 1, 10 );
  rtc_getTime(&dt);
  powerRTC( 0, 1 );

  lastMinLogCycle = lastMinNetCycle = rtc_getMinutes(&dt);
  curMinLogCycle = lastMinLogCycle;
  curMinNetCycle = lastMinNetCycle;

#ifdef DEBUG_SLEEP_CYCLE
	D("INIT: curMinLogCycle - lastMinLogCycle >= MaxMinLogCycle ");
	D(curMinLogCycle); D(" "); Dln(lastMinLogCycle);

#define DBGSLEEP	D("cntSleepCycle: ");D(cntSleepCycle);D(" curSleepCycle: ");D(curSleepCycle); \
	D(" maxSleepCycle: ");D(maxSleepCycle);D(" curMinLogCycle: ");D(curMinLogCycle); \
	D(" lastMinLogCycle: ");D(lastMinLogCycle); D(" curMinNetCycle: ");D(curMinNetCycle); \
	D(" lastMinNetCycle: ");Dln(lastMinNetCycle);

#else
#define DBGSLEEP
#endif	/* DEBUG_SLEEP_CYCLE */

}

#define BUF_SIZE	32

void doMaintenance()
{
	char *c;
	char buf[BUF_SIZE];

		memset(buf, 0, BUF_SIZE);
		c = buf;
		while(1) {
			while( Serial.available() ) {
				if(strlen(buf) < BUF_SIZE-1)
					*c++ = Serial.read();
			}
		}
}



void loop()
{
//  float f1, f2;
//  unsigned long f3a, f3b;
  datablock_t	db;


#define DEBUG_SLEEP_CYCLE

	setupLoop();

	/* enter endless loop */
  	while(1) {
  		mySleep();

  		if(Serial.available()) {
  			switch( Serial.read() ) {
  				case '+': /* enter maintenance mode */
  					doMaintenance();
  					break;
					default: ;
				}
			}
	
  		curSleepCycle++;

//  		DBGSLEEP;
		
#ifdef DEBUG_SLEEP_CYCLE
  		D("[");D(cntSleepCycle);D("] ");
  		
  		D("curSleepCycle ");Dln(curSleepCycle);
#endif

#ifndef DEBUG_SLEEP_CYCLE
			Serial.write(".");
#endif


		if(curSleepCycle >= maxSleepCycle) {
			
#if 1			
			/* do not reset curSleepCycle now, but leave it as is.
			 * the reason is that one minute is 60 seconds, but with 8seconds
			 * per sleep cycle one needs 7.5 sleep cycles, so we should set
			 * maxSleepCycle to 7, then in 4 seconds one minute will have passed,
			 * if we reset the curSleepCycle here, in order for the algorithm to
			 * sense for the minute passed must wait for another 56 seconds.
			 * If we leave curSleepCycle as is, in the next 8 seconds, the algorithm
			 * will sense the passed minute and reset the timer. This results in
			 * timing */
			curSleepCycle = 0;
#endif
	
#ifdef DEBUG_SLEEP_CYCLE
			D(F("curSleepCycle >= maxSleepCycle "));Dln(curSleepCycle);
#endif			
			db.nodeId = NODE_ID;
			db.batVolt = readVcc();
			
			powerRTC( 1, 10 );
//			displayTime();
			rtc_getTime(&dt);
			powerRTC( 0, 1 );
			
			curMinLogCycle = curMinNetCycle = rtc_getMinutes(&dt);

#ifdef DEBUG_SLEEP_CYCLE
			D(F("logCycle remaining: "));D(maxMinLogCycle - (curMinLogCycle - lastMinLogCycle)); D(F("\tnetCycle remaining: ")); Dln(maxMinNetCycle - (curMinNetCycle - lastMinNetCycle));
#endif

			if(abs(curMinLogCycle - lastMinLogCycle) >= maxMinLogCycle) {
					/* maxMinLogCycle minutes have passed since last cycle
					 * do log-ging of data */

#ifdef DEBUG_SLEEP_CYCLE
			    D( ("curMinLogCycle - lastMinLogCycle >= MaxMinLogCycle ") );
			    D(curMinLogCycle); D(" "); Dln(lastMinLogCycle);
#endif

			    lastMinLogCycle = curMinLogCycle;

#if 0
					/* this is the correct position to reset curSleepCycle */
			    curSleepCycle = 0;	/* reset sleep cycle */
#endif

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

				/* disable local echo */
				gsm_sendcmd(CF("ATE0\r\n"));

				if ( gsm_sendPin( CF( "1234" ) ) ) {
					Dln( F("PIN set OK") );
				} else {
					Dln( F("PIN was NOT set") );
				}

				if(!gsm_sendrecvcmdtimeout( CF(""), CF("SMS Ready\r\n"), 10)) {
					Dln("module out of sync....");
				} else Dln("module in sync....");
				

				
		
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

				if( gsm_dnsLookup("internet.cyta.gr","","",NULL, serverip)) {
					if(gsm_activateBearerProfile("internet.cyta.gr", "", "")) {
						if( http_initiateGetRequest() ) {
							if( http_send_datablock( db ) ) {
							} else Serial.println(F("error: could not send data block"));
						} else Serial.println(F("error: could not initiate get request"));
					} else Serial.println(F("error: could not activate bearer profile"));
				} else Serial.println(F("error: could not resolve server ip dns name"));
				
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

