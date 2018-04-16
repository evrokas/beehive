/*
 * main.cpp - main file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-16-17-18
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#include <Arduino.h>
#include <Wire.h>
//#include <EEPROM.h>

#include "conf.h"
#include "bms.h"
#include "utils.h"
#include "rtc.h"
#include "accel.h"
#include "thermal.h"
#include "LowPower.h"
#include "gsm.h"
//#include "data.h"
#include "mem.h"


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


#define A_ENABLE			0x01				/* enable module, enable means that it actually performs logging and net traffic,
																	 * otherwise, it is in a low power state, during which it sleeps for 8 sec and
																	 * wakes up to see what happens */
#define A_DNSLOOKUP		0x02				/* enable DNS look up, this is enabled only when APN is given in a form that needs
																	 * resolving, if it is given in numeric format, do not perform DNS lookup */

uint16_t moduleAflags = 0;



#ifndef NODE_ID

#define NODE_ID	1088

#endif


#if 1
/* addr* variables hold the address in EEPROM of the corresponding
 * variables, but these cost valuable RAM memory, definition could
 * be in uint16_t, but would cost 2 bytes for each variable, so
 * it is changed to uint8_t, so only 1 byte is used
 * IMPORTANT make sure no more than 256 bytes of memory is allocated
 * this way, because address would overflow */
 
uint8_t	addrNodeId;
uint8_t	addrAPIKEY;
uint8_t	addrAPN;
uint8_t	addrUSER;
uint8_t	addrPASS;
uint8_t	addrURL;
uint8_t	addrPORT;
uint8_t addrLogCycle;
uint8_t addrNetCycle;
uint8_t addrSIMPIN;
uint8_t addrSIMICCID;
uint8_t addrVCCfactor;

#endif

#define APIKEY_SIZE		8
#define APN_SIZE			24
#define USER_SIZE			8
#define PASS_SIZE			8
#define URL_SIZE			24
#define SIMPIN_SIZE			4
#define SIMICCID_SIZE		20

#define EEPROM_MAXSTR	24		/* set to maximum length of the above */

#define E_URL		1
#define E_APN		2
#define E_USER	3
#define E_PASS	4
#define E_APIKEY		5
#define E_SIMPIN		6
#define E_SIMICCID	7


void initializeEEPROM()
{
	Serial.print(F("Initializing EEPROM memory allocation ... "));
	addrNodeId	= eepromGetAddr( 2 );
	addrAPIKEY	= eepromGetAddr( 8 );
	addrAPN			= eepromGetAddr( APN_SIZE );
	addrUSER		= eepromGetAddr( USER_SIZE );
	addrPASS		= eepromGetAddr( PASS_SIZE );
	addrURL			= eepromGetAddr( URL_SIZE );			
	addrPORT		= eepromGetAddr( 2 );			/* 0-65535, 2 bytes long */
	addrLogCycle = eepromGetAddr( 2 );
	addrNetCycle = eepromGetAddr( 2 );
	addrSIMPIN	 = eepromGetAddr( SIMPIN_SIZE );
	addrSIMICCID	= eepromGetAddr( SIMICCID_SIZE );
	addrVCCfactor	= eepromGetAddr( 4 );
	
	Serial.print( eepromGetLastAddr() );
	Serial.println( F(" bytes") );
}



uint16_t getNodeId()
{
  return ( eepromGetWord( addrNodeId ) );
}

void setNodeId(uint16_t nodeid)
{
	eepromSetWord( addrNodeId, nodeid );
}

uint8_t getEEPROMstr(uint8_t ecode, char *dat)
{
	switch( ecode ) {
		case E_URL: eepromGetStr(addrURL, URL_SIZE, dat); break;
		case E_APN: eepromGetStr(addrAPN, APN_SIZE, dat); break;
		case E_USER: eepromGetStr(addrUSER, USER_SIZE, dat); break;
		case E_PASS: eepromGetStr(addrPASS, PASS_SIZE, dat); break;
		case E_APIKEY: eepromGetStr(addrAPIKEY, APIKEY_SIZE, dat); break;
		case E_SIMPIN: eepromGetStr(addrSIMPIN, SIMPIN_SIZE, dat); break;
		case E_SIMICCID: eepromGetStr(addrSIMICCID, SIMICCID_SIZE, dat); break;
	default:
		/* if wrong ecode then return fail */
		return 0;
	}
	return 1;
}

uint8_t setEEPROMstr(uint8_t ecode, char *dat)
{
	switch( ecode ) {
		case E_URL: eepromSetStr(addrURL, URL_SIZE, dat); break;
		case E_APN: eepromSetStr(addrAPN, APN_SIZE, dat); break;
		case E_USER: eepromSetStr(addrUSER, USER_SIZE, dat); break;
		case E_PASS: eepromSetStr(addrPASS, PASS_SIZE, dat); break;
		case E_APIKEY: eepromSetStr(addrAPIKEY, APIKEY_SIZE, dat); break;
		case E_SIMPIN: eepromSetStr(addrSIMPIN, SIMPIN_SIZE, dat); break;
		case E_SIMICCID: eepromSetStr(addrSIMPIN, SIMICCID_SIZE, dat); break;
	default:
		/* if wrong ecode, then return fail */
		return 0;
	}
	return 1;
}


void setLogCycle(uint8_t dat)
{
	eepromSetByte( addrLogCycle, dat );
}

uint8_t getLogCycle()
{
	return( eepromGetByte( addrLogCycle ) );
}

void setNetCycle(uint8_t dat)
{
	eepromSetByte( addrNetCycle, dat );
}

uint8_t getNetCycle()
{
  return( eepromGetByte( addrLogCycle ) );
}

uint16_t getServerPort()
{
	return ( eepromGetWord( addrPORT ) );
}

void setServerPort( uint16_t dat )
{
	eepromSetWord( addrPORT, dat );
}

void setVCC( float dat )
{
	eepromSetFloat( addrVCCfactor, dat );
}

float getVCC()
{
  return (eepromGetFloat( addrVCCfactor ));
}


void loadVariablesFromEEPROM()
{
	Serial.print(F("Node ID: ")); Serial.println( getNodeId() );
	setVccFactor( getVCC() );
	
	maxMinLogCycle = getLogCycle();
	maxMinNetCycle = getNetCycle();
	Serial.print(F("Log cycle: ")); Serial.print(maxMinLogCycle);
	Serial.print(F("\tNet cycle: ")); Serial.println(maxMinNetCycle);
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


#if 0
 	maxMinLogCycle = DAILY_LOG_PERIOD;
	maxMinNetCycle = DAILY_NET_PERIOD;
#endif

	loadVariablesFromEEPROM();

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
	delay(500);
	
	Dinit;
	Serial.begin( 9600 );

	Serial.println(	F("Beehive Monitoring System (c) 2015,16,17,18. All Rights reserved"));
	Serial.println(	F("(c) Evangelos Rokas <evrokas@gmail.com>"));
	Serial.print(		F("System Board version ")); Serial.print( BOARD_REVISION );
	Serial.print(		F("  Firmware version ")); Serial.println( FIRMWARE_REVISION );
	
//	delay(2000);

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

 	initializeEEPROM();

	initializeCounters(); 
      
	setupPeripheralsControl();

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

#if ENABLE_DATAPUSHING == 1
	mem_init( EXT_EEPROM_SIZE, EXT_EEPROM_ADDR );
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

//	setNodeId( NODE_ID );
}


/* this is where the actual sleep of the mcu takes place, try
 * to keep this as small as possible */
void mySleep()
{
	Serial.flush();
	LowPower.powerDown(LP_SLEEP_MODE, ADC_OFF, BOD_OFF);
	Wire.begin();

//	Serial.begin( 9600 );	

	/* increase sleep cycle counter */
	cntSleepCycle++;
	
//	TWBR = 152;		/* switch to 25KHz I2C interface */
}


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
	Dp("INIT: curMinLogCycle - lastMinLogCycle >= MaxMinLogCycle ");
	D(curMinLogCycle); Dp(" "); Dln(lastMinLogCycle);

#define DBGSLEEP	Dp("cntSleepCycle: ");D(cntSleepCycle);Dp(" curSleepCycle: ");D(curSleepCycle); \
	Dp(" maxSleepCycle: ");D(maxSleepCycle);Dp(" curMinLogCycle: ");D(curMinLogCycle); \
	Dp(" lastMinLogCycle: ");D(lastMinLogCycle); Dp(" curMinNetCycle: ");D(curMinNetCycle); \
	Dp(" lastMinNetCycle: ");Dln(lastMinNetCycle);

#else
#define DBGSLEEP
#endif	/* DEBUG_SLEEP_CYCLE */

}

#define BUF_SIZE	EEPROM_MAXSTR		/* was 16 */

bool doNet=false, doLog=false;


#if ENABLE_MAINTENANCE == 1
void doMaintenance()
{
	char *c,cc;
	char buf[BUF_SIZE+1];


		Serial.print( F(">") );

		while(1) {
			memset(buf, 0, BUF_SIZE);
			c = buf;

			Serial.print( F("> ") );
			
			while(1) {
				if(Serial.available() ) {
					cc = Serial.read();
						
					/* ignore all + symbols */
					if(cc == '+')continue;
						
					/* if \r is received, execute the command */
					if( cc == '\r') break;
					
					if( cc == '\h') {/* back up */
						if(strlen(buf) > 0) {
							Serial.write( cc );
							*c = '\0';
							c--;
						}
					}
					else
					/* otherwise add cc to the buffer and continue */
					if(strlen(buf) < BUF_SIZE) {
						*c++ = cc;

						Serial.write( cc );
//					  Serial.println(buf);
					}
				}
			}
			
#if 0
			Serial.print(F("execute command :")); Serial.println( buf );
#endif
	
			if(strlen(buf) > 0) {
				switch(buf[0]) {
					case '?':
						Serial.println(F("Help message!"));
						break;
					
					case 'u':
						if(buf[1] == '?') {
							Serial.print(F("server url: "));
							memset(buf, 0, BUF_SIZE+1);
							if(getEEPROMstr( E_URL, buf ))
								Serial.println( buf );
							break;
						};
						if(setEEPROMstr( E_URL, buf ))
							Serial.println(F("url set ok!"));
						break;
					
					case 'r':
						if(buf[1] == '?') {
							Serial.print(F("server port: "));
							Serial.println( getServerPort() );
							break;
						} else {
						  uint16_t n;

								n = atoi( buf+1 );
								setServerPort( n );
								Serial.println(F("server port set ok!"));
						}
						break;
					
					case 'a':
						if(buf[1] == '?') {
							Serial.print(F("APN: "));
							memset(buf, 0, BUF_SIZE+1);
							if(getEEPROMstr( E_APN, buf ))
								Serial.println( buf );
							break;
						};
						if(setEEPROMstr( E_APN, buf+1 ))
							Serial.println(F("apn set ok!"));
						break;

					case 's':
						if(buf[1] == '?') {
							Serial.print(F("USER: "));
							memset(buf, 0, BUF_SIZE+1);
							if(getEEPROMstr( E_USER, buf ))
								Serial.println( buf );
							break;
						};
						if(setEEPROMstr( E_USER, buf ))
							Serial.println(F("user set ok!"));
						break;

					case 'w':
						if(buf[1] == '?') {
							Serial.print(F("PASS: "));
							memset(buf, 0, BUF_SIZE+1);
							if(getEEPROMstr( E_PASS, buf ))
								Serial.println( buf );
							break;
						};
						if(setEEPROMstr( E_PASS, buf ))
							Serial.println(F("pass set ok!"));
						break;

					case 'i':
						if(buf[1] == '?') {
							Serial.print(F("Node ID: ")); Serial.println( getNodeId() );
							break;
						} else {
							int n;
							
								n = atoi( buf+1 );
								if(n>0 && n < 4096) {
									Serial.print(F("New Node ID : ")); Serial.println(n);
									setNodeId( n );
								}
						}
						break;

					case 'k':
						if(buf[1] == '?') {
							Serial.print(F("APIKEY: "));
							memset(buf, 0, BUF_SIZE+1);
							if(getEEPROMstr( E_APIKEY, buf ))
								Serial.println( buf );
							break;
						};
						if(setEEPROMstr( E_APIKEY, buf ))
							Serial.println(F("apn set ok!"));
						break;

					case 'l':
						if(buf[1] == '?') {
							Serial.print(F("Log frequency: ")); Serial.println( maxMinLogCycle );
							break;
						} else {
							uint16_t n;
								n = atoi( buf+1 );
								Serial.print(F("New log frequency: ")); Serial.println( n );
								maxMinLogCycle = n;
								setLogCycle( n );
						}
						break;
						
					case 'n':
						if(buf[1] == '?') {
							Serial.print(F("Net frequency: ")); Serial.println( maxMinNetCycle );
							break;
						} else {
							uint16_t n;
								n = atoi( buf+1 );
								Serial.print(F("New net frequency: ")); Serial.println( n );
								maxMinNetCycle = n;
								setNetCycle( n );
						};
						break;							

					case 'p':
						if(buf[1] == '?') {
							Serial.print(F("SIM PIN: "));
							memset(buf, 0, BUF_SIZE+1);
							if(getEEPROMstr( E_SIMPIN, buf ))
								Serial.println( buf );
							break;
						};
						if(setEEPROMstr( E_SIMPIN, buf ))
							Serial.println(F("sim pin set ok!"));
						break;

					case 'd':
						if(buf[1] == '?') {
							Serial.print(F("SIM ICIID: "));
							memset(buf, 0, BUF_SIZE+1);
							if(getEEPROMstr( E_SIMICCID, buf ))
								Serial.println( buf );
							break;
						};
						if(setEEPROMstr( E_SIMICCID, buf ))
							Serial.println(F("sim iccid set ok!"));
						break;

					case 'v':
						if(buf[1] == '?') {
							Serial.print(F("VCC factor: "));
							Serial.println( getVccFactor() );
						} else {
						  float f;
						  	
						  	f = atof( buf+1 );
						  	Serial.print(F("New VCC factor: "));
						  	Serial.println( f );
						  	setVccFactor( f );
								setVCC( f );
						}
						break;
							
					case 'X':	/* exit maintenance mode */;
						return;
						break;

					case 'E':	/* reset counter to initial values */
						/* TODO */
						break;

					case 'P': /* print sleep counter */
						Serial.print(F("Sleep cycle counter: ")); Serial.print(curSleepCycle);
						Serial.print(F("  sleep counter: ")); Serial.println( cntSleepCycle );
						break;
					
					case 'D':	/* dump data values */
						/* TODO */
						break;

					case 'V':	/* print battery voltage */
						Serial.print(F("Battery voltage: ")); Serial.println(readVcc() / 1000.0 );
						break;

					case 'T':
						powerPeripherals(1,100);
						Serial.print(F("Temperature (oC): ")); Serial.println( therm_getTemperature() *100 );
						powerPeripherals(0,0);
						break;
						
					case 'H':
						powerPeripherals(1,100);
						Serial.print(F("Humidity (%%): ")); Serial.println( therm_getHumidity() * 100 );
						powerPeripherals(0,0);
						break;
						
					case 'C':
						powerRTC(1, 10);
						Serial.print(F("date time: ")); displayTime();
						powerRTC(0, 1);
						break;
					
					case 'G':
						switch( buf[1] ) {
							case '0': powerGPRSGPS( 0 ); break;
							case '1': powerGPRSGPS( 1 ); break;
						
							default:
								Serial.println(F("wrong G command!"));
						}
						
					case 'B': {
						uint16_t n;
							Serial.print(F("GSM module battery: "));
							if( gsm_getBattery( n ) )
								Serial.println( n );
							else Serial.println(F("error"));
						}
						break;
					
					case 'Q': {
					  uint8_t n;
					  	Serial.print(F("GSM signal quality: "));
					  	if( gsm_getSignalQuality( n ) )
					  		Serial.println( n );
							else Serial.println( F("error"));
						}
						break;
					
					case 'R':
						if( buf[1] == '?' ) {
							uint8_t r;

								if( gsm_getRegistration( r ) ) {
									Serial.print(F("GSM/GPRS module registration: "));
									switch( r ) {
										case 0: Serial.println(F("not registered, currently not searching for operator (0)")); break;
										case 1: Serial.println(F("registrered, home network (1)")); break;
										case 2: Serial.println(F("not registered, currently seeking for operator (2)")); break;
										case 3: Serial.println(F("registration denied (3)")); break;
										case 4: Serial.println(F("unknown (4)")); break;
										case 5: Serial.println(F("registered, roaming (5)")); break;
									}
								} else {
									Serial.println(F("Could not determine GSM/GPRS module registration status"));
									break;
								}
						} else
						if(buf[1] == '0' ) {
							/* deregister module from network */
						} else
						if(buf[1] == '1' ) {
							/* register module in network */
						}
						
						/* TODO */
						break;

					case 'F':
						Serial.print( F("Firmware version: ")); Serial.println(F( FIRMWARE_REVISION_LONG ) );
						break;

					case 'A':
						switch( buf[1] ) {
							case '?':
								Serial.println(F("Current flags:"));
								if(moduleAflags & A_ENABLE)Serial.println(F("Module enabled"));
								else Serial.println(F("Module not enabled"));
								if(moduleAflags & A_DNSLOOKUP)Serial.println(F("DNS lookup active"));
								else Serial.println(F("DNS lookup not active"));
								break;
							case 'a':
								switch( buf[2] ) {
									case '0': moduleAflags &= ~A_ENABLE; break;
									case '1': moduleAflags |= A_ENABLE; break;
									default:
										Serial.println(F("wrong parameter!"));
										break;
								}
								break;
							case 'l':
								switch( buf[2] ) {
									case '0': moduleAflags &= ~A_DNSLOOKUP; break;
									case '1': moduleAflags |= A_DNSLOOKUP; break;
									default:
										Serial.println(F("wrong parameter!"));
										break;
								}
								break;
							default:
								Serial.println(F("wrong parameter"));
								break;
						}
						break;

					case 'O':
						/* print tail and head pointers */
						Serial.print(F("db pointers:\thead: ")); Serial.print( __head_db );
						Serial.print(F("\ttail: ")); Serial.println( __tail_db );
						break;

					default:
						Serial.println(F("unknown command!"));
						break;
					}
			}
			
		}
}
#endif



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


#if ENABLE_MAINTENANCE == 1
	{
		int cnt32 = 10;
		
//  		Serial.println( F("out of sleep") );
//  		delay(2);
  		while(Serial.available() || (cnt32--)) {
  			switch( Serial.read() ) {
  				case '+': /* enter maintenance mode */
  					while(Serial.available())Serial.read();
  					Serial.println( F("entering maintanance mode!") );
						
						/* since we are going to send A LOT of '+', eat them all! */
						while(Serial.available() && (Serial.read() == '+')) ;
						
  					doMaintenance();
  					break;
					default:
						//delay(1);
						Serial.print( F(".") );
						break;
				}
			}
	}
#endif

	
  		curSleepCycle++;

//  		DBGSLEEP;
		
#ifdef DEBUG_SLEEP_CYCLE
#if 0
  		Dp("[");D(cntSleepCycle);Dp("] ");
  		
  		Dp("curSleepCycle ");Dln(curSleepCycle);
#endif

			Dp("[");D(cntSleepCycle);Dp("]");
			
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
#if 0
			Dp("curSleepCycle >= maxSleepCycle ");Dln(curSleepCycle);
#endif
#endif			
			db.nodeId = getNodeId();		//NODE_ID;
			db.batVolt = readVcc();
			
			powerRTC( 1, 10 );
//			displayTime();
			rtc_getTime(&dt);
			powerRTC( 0, 1 );
			
			curMinLogCycle = curMinNetCycle = rtc_getMinutes(&dt);

#ifdef DEBUG_SLEEP_CYCLE
#if 0
			Dp("logCycle remaining: ");D(maxMinLogCycle - (curMinLogCycle - lastMinLogCycle)); Dp("\tnetCycle remaining: "); Dln(maxMinNetCycle - (curMinNetCycle - lastMinNetCycle));
#endif
			Dp("{");D(maxMinLogCycle - (curMinLogCycle - lastMinLogCycle));Dp(",");D(maxMinNetCycle - (curMinNetCycle - lastMinNetCycle));Dlnp("}");
#endif

			doNet = doLog = false;
			
			if(abs(curMinLogCycle - lastMinLogCycle) >= maxMinLogCycle)
				doLog = true;

			if(doLog) {	//abs(curMinLogCycle - lastMinLogCycle) >= maxMinLogCycle) {
					/* maxMinLogCycle minutes have passed since last cycle
					 * do log-ging of data */

#ifdef DEBUG_SLEEP_CYCLE
			    Dp("curMinLogCycle - lastMinLogCycle >= MaxMinLogCycle ");
			    D(curMinLogCycle); Dp(" "); Dln(lastMinLogCycle);
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
					db.bhvWeight = 75432;
								    
			    powerPeripherals(0,0);
			    powerRTC(0, 0);
			    doLog = false;
			    
					db.rtcDateTime = unixtime( &dt );
					memcpy((void *)&db.dt, (void *)&dt, sizeof(dt));
					
//					Dln("after powering off peripherals");

#define DEB_STUFF

#ifdef DEB_STUFF
          Serial.print( F(">>,") );

		    	/* power voltage, before powering peripherals */
		    	Serial.print(db.batVolt); Serial.print( F(",") );
			
		    	/* temperature */
		    	Serial.print(db.bhvTemp); Serial.print( F(",") );
			
					/* humidity */
		    	Serial.print(db.bhvHumid); Serial.print( F("\n\r") );

		    	Serial.flush();
#endif


#if ENABLE_DATAPUSHING == 1

					if( mem_pushDatablock( &db ) ) {
						Serial.println( F("pushed datablock to EEPROM successfully.") );
					} else {
						Serial.println( F("could not push datablock to EEPROM.") );
					}
#endif
					
					/* put here code to store in EEPROM memory data blocks until
					 * net cycle is reached, and data are forwarded to internet server */
			}
  
			if(abs(curMinNetCycle - lastMinNetCycle) >= maxMinNetCycle)
				doNet = true;
				
			if(doNet)	{	//abs(curMinNetCycle - lastMinNetCycle) >= maxMinNetCycle) {
				/* maxMinNetCycle minutes have passed since last cycle
				 * do net-working stuff */

#ifdef DEBUG_SLEEP_CYCLE
			    Dp("curMinNetCycle - lastMinNetCycle >= MaxMinNetCycle ");
			    D(curMinNetCycle); Dp(" "); Dln(lastMinNetCycle);
#endif

#if 0
				if( poweredGSM >= 4 ) {
					Dlnp("GSM was not ready after 3 tries. Skip NETWORK session");
					
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
					Dp("Trying to power-up GSM/GPRS module - try "); Dln( poweredGSM );
					
					/* only apply power on the first time */
					if(poweredGSM == 1)
						powerGPRSGPS( 1 );

				}

				//poweredGSM++;
				
				while( poweredGSM < 10 ) {
					if( !gsm_moduleInfo() ) {
						Dp("GSM module is not ready yet! try ... "); Dln( poweredGSM );
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
				
				Dlnp("GSM module is ready!");
				/* so module is ready */

				Dlnp("Doing network stuff");

				/* disable local echo */
				gsm_sendcmd(CF("ATE0\r\n"));

				if ( gsm_sendPin( CF( "1234" ) ) ) {
					Dlnp("PIN set OK");
				} else {
					Dlnp("PIN was NOT set");
				}

				if(!gsm_sendrecvcmdtimeout( CF(""), CF("SMS Ready\r\n"), 10)) {
					Dlnp("module out of sync....");
				} else Dlnp("module in sync....");
				

				
#define REG_TIMEOUT	30
		
				do {
					uint8_t r, cc=0;
						do {
							gsm_getRegistration( r );
							if((r == 1) || (r == 5))break;
							delay( 200 );
							cc++;
						} while( cc < REG_TIMEOUT );		/* set a timeout here, 15 secs */
				
						if( cc == REG_TIMEOUT ) { 
							poweredGSM = 0;
							powerGPRSGPS( 0 );

							lastMinNetCycle = curMinNetCycle;
							curSleepCycle = 0;	/* reset sleep cycle */

							doNet = 0;
							continue;
						}
				} while(0);
				
				
				do {
					uint16_t ii;
					uint8_t iii;
					
						if( gsm_getBattery( ii ) ) {
//							Serial.print("Battery level: " ); Serial.println( ii );
							db.gsmVolt = ii;
						}

						if( gsm_getSignalQuality( iii ) ) {
//							Serial.print("Signal quality: " ); Serial.println( iii );
							db.gsmSig = iii;
						}
				} while(0);

				do {
					uint8_t srvip[4];
						
						if( gsm_dnsLookup(CF( APN ),CF( USERNAME ),CF( PASSWORD ),CF( DBSERVER ), NULL, srvip)) {
							Dlnp("successfully resolved domain name");
							memcpy(serverip, srvip, 4);
						} else {
							Dlnp("could not successfully resolve domain name, using old IP address");
						}
				} while(0);
				
				if(gsm_activateBearerProfile(CF( APN ), CF( USERNAME ), CF( PASSWORD ))) {
					if( http_initiateGetRequest() ) {
						if( http_send_datablock( db ) ) {
						} else Serial.println( F("error: could not send data block"));
					} else Serial.println( F("error: could not initiate get request"));
				} else Serial.println( F("error: could not activate bearer profile"));
				
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
	
	}	/* main while logop  */
}

