/*
 * main.cpp - main file
 *
 * BeeWatch Project
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-18
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 */

#include <avr/wdt.h>

#include <Arduino.h>
#include <Wire.h>
#include <NeoSWSerial.h>

#include "conf.h"
#include "ver.h"

#include "pstr.h"
#include "bms.h"
#include "utils.h"
#include "rtc.h"
#include "accel.h"
#include "thermal.h"
#include "LowPower.h"
#include "gsm.h"
#include "mem.h"
#include "error.h"


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

uint16_t	timeoutLogCycle;
uint16_t	timeoutNetCycle;


#define A_ENABLE			0x01				/* enable module, enable means that it actually performs logging and net traffic,
																	 * otherwise, it is in a low power state, during which it sleeps for 8 sec and
																	 * wakes up to see what happens */

#define A_DNSLOOKUP		0x02				/* enable DNS look up, this is enabled only when APN is given in a form that needs
																	 * resolving, if it is given in numeric format, do not perform DNS lookup */

#define A_DST					0x04				/* Daylight Savings Time bit, if 1, add 1 hour to time */


uint16_t moduleAflags = 0;

#define isModuleEnabled			(moduleAflags & A_ENABLE)
#define isDNSLookupEnabled	(moduleAflags & A_DNSLOOKUP)
#define isDSTEnabled				(moduleAflags & A_DST)


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
uint8_t addrAflags;
#endif



void initializeEEPROM()
{
	Serial.print(F("Initializing EEPROM memory allocation ... "));
	addrNodeId	= eepromGetAddr( 2 );			/* 0-65535, 2 bytes long */
	addrAPIKEY	= eepromGetAddr( APIKEY_SIZE );
	addrAPN			= eepromGetAddr( APN_SIZE );
	addrUSER		= eepromGetAddr( USER_SIZE );
	addrPASS		= eepromGetAddr( PASS_SIZE );
	addrURL			= eepromGetAddr( URL_SIZE );			
	addrPORT		= eepromGetAddr( 2 );			/* 0-65535, 2 bytes long */
	addrLogCycle = eepromGetAddr( 2 );		/* log cycle, every # minutes, 2 bytes long */
	addrNetCycle = eepromGetAddr( 2 );		/* net cycle, every # minutes, 2 bytes long */
	addrSIMPIN	 = eepromGetAddr( SIMPIN_SIZE );
	addrSIMICCID	= eepromGetAddr( SIMICCID_SIZE );
	addrVCCfactor	= eepromGetAddr( 4 );		/* floating point value */
	addrAflags = eepromGetAddr( 2 );			/* module flags, 2  bytes long  */
	
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

uint16_t getAflags()
{
	return ( eepromGetWord( addrAflags ) );
}

void setAflags(uint16_t dat)
{
	eepromSetWord( addrAflags, dat);
}


char *getEEPROMstr(uint8_t ecode, char *dat)
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
		return NULL;
	}
	return dat;
}

char *setEEPROMstr(uint8_t ecode, char *dat)
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
		return NULL;
	}
	return dat;
}

bool transmitEEPROMstr(uint8_t ecode, Stream &strm, bool debugSerial)
{
	uint8_t i,m, sta;
	char c;
	
	switch( ecode ) {
		case E_URL: m = URL_SIZE; sta = addrURL; break;
		case E_APN: m = APN_SIZE; sta = addrAPN; break;
		case E_USER: m = USER_SIZE; sta = addrUSER; break;
		case E_PASS: m = PASS_SIZE; sta = addrPASS; break;
		case E_APIKEY: m = APIKEY_SIZE; sta = addrAPIKEY; break;
		case E_SIMPIN: m = SIMPIN_SIZE; sta = addrSIMPIN; break;
		case E_SIMICCID: m = SIMICCID_SIZE; sta = addrSIMICCID; break;
		default: return (false);	//m = 0; break;
	}
	
	for(i=0;i<m;i++) {
		c = eepromGetByte( sta + i );
		if(c == 0)break;	/* end of string */

		strm.write(c);

		if( debugSerial )
			Serial.write(c);
	}

	return (true);
}

#if 0
bool transmitEEPROMstrd(uint8_t ecode, Stream &strm)
{
	uint8_t i,m, sta;
	char c;
	
	switch( ecode ) {
		case E_URL: m = URL_SIZE; sta = addrURL; break;
		case E_APN: m = APN_SIZE; sta = addrAPN; break;
		case E_USER: m = USER_SIZE; sta = addrUSER; break;
		case E_PASS: m = PASS_SIZE; sta = addrPASS; break;
		case E_APIKEY: m = APIKEY_SIZE; sta = addrAPIKEY; break;
		case E_SIMPIN: m = SIMPIN_SIZE; sta = addrSIMPIN; break;
		case E_SIMICCID: m = SIMICCID_SIZE; sta = addrSIMICCID; break;
		default: return (false);	//m = 0; break;
	}
	
	for(i=0;i<m;i++) {
		c = eepromGetByte( sta + i );
		if(c == 0)break;	/* end of string */

		strm.write(c);
		Serial.print(c);
	}

	return (true);
}
#endif	

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
  return( eepromGetByte( addrNetCycle ) );
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
	moduleAflags = getAflags();
	
	Serial.print(RCF( pNodeId )); Serial.println( getNodeId() );
	setVccFactor( getVCC() );
	
	maxMinLogCycle = getLogCycle();
	maxMinNetCycle = getNetCycle();
	Serial.print(F("Log cycle: ")); Serial.print(maxMinLogCycle);
	Serial.print(F("\tNet cycle: ")); Serial.println(maxMinNetCycle);
}

void dumpEEPROMvariables()
{
	Serial.println(F("EEPROM variables:"));
	Serial.print(RCF( pNodeId )); Serial.println( getNodeId() );
	
	Serial.print(F("APIKEY: \"")); 
	transmitEEPROMstr(E_APIKEY, Serial);
	Serial.println(F("\""));
	
	Serial.print(F("APN: \"")); 
	transmitEEPROMstr(E_APN, Serial);
	Serial.println(F("\""));
	
	Serial.print(F("USER: \""));
	transmitEEPROMstr(E_USER, Serial);
	Serial.println(F("\""));
	
	Serial.print(F("PASS: \""));
	transmitEEPROMstr(E_PASS, Serial);
	Serial.println(F("\""));
	
	Serial.print(F("URL: \""));
	transmitEEPROMstr(E_URL, Serial);
	Serial.println(F("\""));
	
	Serial.print(F("PORT: ")); Serial.println( getServerPort() );
	Serial.print(F("Log Cycle: ")); Serial.println( getLogCycle() );
	Serial.print(F("Net Cycle: ")); Serial.println( getNetCycle() );

	Serial.print(F("SIM PIN: \""));
	transmitEEPROMstr(E_SIMPIN, Serial);	
	Serial.println(F("\""));
	
	Serial.print(F("SIM ICCID: \""));
	transmitEEPROMstr(E_SIMICCID, Serial);
	Serial.println(F("\""));

	Serial.print(F("VCC factor: ")); Serial.println( getVCC(), 5 );

	Serial.print(F("Module flags: "));
		if(isModuleEnabled)Serial.print(F("ModEn\t"));
			else Serial.print(F("ModDis\t"));
		if(isDNSLookupEnabled)Serial.print(F("DNSEn\t"));
			else Serial.print(F("DNSDis\t"));
		if(isDSTEnabled)Serial.print(F("DSTEn\t"));
			else Serial.print(F("DSTDis\t"));
	Serial.println();
	
	Serial.println( F("------------") );
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


	timeoutLogCycle = 0;
	timeoutNetCycle = 0;


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


void setupLoop();

/* setup steps of the project */
void setup()
{
	/* initialize peripheral control as early as possible, in order
	 * to turn off GSM module, which is by default turned on(!!!) */
	setupPeripheralsControl();

	delay(500);
	
	Dinit;
	Serial.begin( 9600 );

	Serial.println(	F("Beehive Monitoring System (c) 2015-18. All Rights reserved"));
	Serial.println(	F("(c) Evangelos Rokas <evrokas@gmail.com>"));
	Serial.print(		F("System Board version ")); Serial.print( F(BOARD_REVISION) );
	Serial.print(		F("  Firmware version ")); Serial.println( F(FIRMWARE_REVISION) );
	
	Serial.print(   F("Date: ")); Serial.print( F(FIRMWARE_DATE) );
	Serial.print( 	F("  upload no # ")); Serial.println( F(FIRMWARE_UNUM) );

	Serial.print( F("Utilizing "));
#ifdef HTTP_API_POST
	Serial.print( F("POST") );
#endif
#ifdef HTTP_API_GET
	Serial.print( F("GET") );
#endif
	Serial.println( F(" method") );
	

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

 	dumpEEPROMvariables();

	initializeCounters(); 
      
	powerPeripherals(1,1);

	/* GSM is turned off by initialize function */
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

	mem_init( EXT_EEPROM_SIZE, EXT_EEPROM_ADDR );

	initErrorHandling();

	powerPeripherals(0,0);	/* disable all peripherals */

	setupLoop();
}


volatile uint8_t rx_interrupt = 0;


extern "C" {

ISR(PCINT2_vect)
{
	rx_interrupt++;

	NeoSWSerial::rxISR( PIND );
}


ISR(PCINT0_vect)
{
	NeoSWSerial::rxISR( PINB );
}

ISR(PCINT1_vect)
{
	NeoSWSerial::rxISR( PINC );
}

};

/* this is where the actual sleep of the mcu takes place, try
 * to keep this as small as possible */
void mySleep()
{
//	wdt_disable();

	Serial.flush();


	/* setup to wake on pin change on D0 (RX) */
	cli();

	PCMSK2	|= bit( PCINT16 );
	PCIFR		|= bit( PCIF2 );
	PCICR		|= bit( PCIE2 );
	//Interrupts();
	sei();
	
	LowPower.powerDown(LP_SLEEP_MODE, ADC_OFF, BOD_OFF);
	
	PCICR		&= ~bit( PCIE2 );
	
	Wire.begin();

//	Serial.begin( 9600 );	

	/* increase sleep cycle counter */
	cntSleepCycle++;
	
	Wire.setClock( 100000 );	//TWBR = 32;		// 100Khz 2152;		/* switch to 25KHz I2C interface */
	
	
//	wdt_enable( WDTO_2S );
}

#if 0
ISR(WDT_vect)
{
	Serial.println(F("watchdog reset"));
	wdt_reset();
}
#endif


uint8_t poweredGSM=0;
datetime_t dt;

void updateTimeouts()
{
  timeoutLogCycle = cntSleepCycle + 60 * getLogCycle() / 8;
  timeoutNetCycle = cntSleepCycle + 60 * getNetCycle() / 8;
  
  Dp("log timeout: "); D( timeoutLogCycle ); Dp("\tnet timeout: "); Dln( timeoutNetCycle );
}



void setupLoop()
{
//	powerRTC( 1, 10 );
//	powerPER_RTC(1, 10 );
	powerPeripherals(1, 10);
  rtc_getTime(&dt);
//  powerRTC( 0, 1 );
//	powerPER_RTC(0, 1 );
	powerPeripherals(0, 0);

  lastMinLogCycle = lastMinNetCycle = rtc_getMinutes(&dt);
  curMinLogCycle = lastMinLogCycle;
  curMinNetCycle = lastMinNetCycle;

  updateTimeouts();
  
#define DEBUG_SLEEP_CYCLE

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

	serverip[0] = 192;
	serverip[1] = 168;
	serverip[2] = 1;
	serverip[3] = 1;

}

#define BUF_SIZE	EEPROM_MAXSTR		/* was 16 */

bool doNet=false, doLog=false;


#if ENABLE_MAINTENANCE == 1
void doMaintenance()
{
	char *c,cc;
	char buf[BUF_SIZE+1];		/* +1 to allow for \0 at the end of the string */


//		Serial.print( F(">") );

		while(1) {
			memset(buf, 0, BUF_SIZE);
			c = buf;

			Serial.print( F("> ") );
			
			while(1) {
				if(Serial.available()) {
					cc = Serial.read();
						
					/* ignore all + symbols */
					if(cc == '+')continue;
						
					/* if \r is received, execute the command */
					if( cc == '\r') break;
					
					if( cc == 8) { /* back up */
						if(strlen(buf) > 0) {
							Serial.write( cc );
							c--;
							*c = '\0';
						}
					}
					else
						/* otherwise add cc to the buffer and continue */
						if(strlen(buf) < BUF_SIZE) {
							*c++ = cc;

							Serial.write( cc );
//						  Serial.println(buf);
						}
				}
				delay(100);
			}
			Serial.println();	
#if 1
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
						if(setEEPROMstr( E_URL, buf+1 ))
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
#if 1
							transmitEEPROMstr( E_APN, Serial );
							Serial.println();
#else
							memset(buf, 0, BUF_SIZE+1);
							if(getEEPROMstr( E_APN, buf ))
								Serial.println( buf );
#endif
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
						if(setEEPROMstr( E_USER, buf+1 ))
							Serial.println(F("user set ok!"));
						break;

					case 'w':
						if(buf[1] == '?') {
							Serial.print(F("PASS: "));
							memset(buf, 0, BUF_SIZE+1);
							if(getEEPROMstr( E_PASS, buf ))
								Serial.println( buf );
						} else {
							if(setEEPROMstr( E_PASS, buf+1 ))
								Serial.println(F("pass set ok!"));
						}
						break;

					case 'i':
						if(buf[1] == '?') {
							Serial.print(RCF( pNodeId )); Serial.println( getNodeId() );
							break;
						} else {
							int n;
							
								n = atoi( buf+1 );
								if(n>0 && n < MAX_NODE_NUMBER) {
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
						} else {
							if(setEEPROMstr( E_APIKEY, buf+1 )) {
								Serial.print(F("apn set to: "));
								Serial.println( buf+1 );
							}
						}
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
						if(setEEPROMstr( E_SIMPIN, buf+1 ))
							Serial.println(F("sim pin set ok!"));
						break;

					case 'd':
						switch (buf[1]) {
							case '?':
								Serial.print(F("SIM ICIID: "));
								memset(buf, 0, BUF_SIZE+1);
								if(getEEPROMstr( E_SIMICCID, buf ))
									Serial.println( buf );
								break;
							case '*':
								if(gsm_getICCID( buf )) {
									Serial.print(F("SIM ICCID: "));
									Serial.println( buf );
								}
								break;
							case '+':
								break;
							default:
								if(setEEPROMstr( E_SIMICCID, buf+1 ))
									Serial.println(F("sim iccid set ok!"));
						}
						break;

					case 'v':
						if(buf[1] == '?') {
							Serial.print(F("VCC factor: "));
							Serial.println( getVccFactor(), 5 );
						} else {
						  float f;
						  	
						  	f = atof( buf+1 );
						  	Serial.print(F("New VCC factor: "));
						  	Serial.println( f, 5 );
						  	setVccFactor( f );
								setVCC( f );
						}
						break;
							
					case 'X':	/* exit maintenance mode */;
						return;
						break;

					case 'E':	/* reset counter to initial values */
						__tail_db = 0;
						__head_db = 0;
						
						//powerPER_RTC(1, 10);
						//powerRTC(1, 10);
						powerPeripherals(1, 10);
						mem_storecounters();
						//powerRTC(0, 1);
						//powerPER_RTC(0, 0);
						powerPeripherals(0, 0);
						Serial.println(F("EEPROM head and tail counters have been reset!"));
						break;

					case 'P': /* print sleep counter */
						Serial.print(F("Sleep cycle counter: ")); Serial.print(curSleepCycle);
						Serial.print(F("  sleep counter: ")); Serial.println( cntSleepCycle );
						break;
					
					case 'D':	/* dump data values */
						if(buf[1]) {
							int c;
							datablock_t d;
							
								//powerRTC(1, 10);
								//powerPER_RTC(1, 10);
								powerPeripherals(1, 10);
								mem_readcounters();

								c = atoi( buf+1 );
								mem_readDatablocki((__tail_db + c) % __max_db, &d);
								//powerPER_RTC(0, 0);
								//powerRTC(0, 1);
								powerPeripherals(0, 0);


								dumpDBrecord(&d);
						} else {
							//powerRTC(1, 10);
							//powerPER_RTC(1, 10);
							powerPeripherals(1, 10);
							mem_readcounters();
							//powerRTC(0, 1);
							//powerPER_RTC(0, 0);
							powerPeripherals(0, 0);
							
							Serial.print(F("head: "));Serial.print(__head_db);
							Serial.print(F("\ttail: ")); Serial.println(__tail_db);
						}
						break;

					case 'U':	/* flush data values */
						//powerRTC(1,10);
						//powerPER_RTC(1, 10);
						powerPeripherals(1, 10);
						mem_readcounters();
						__tail_db = __head_db;
						mem_storecounters();
						//powerRTC(0, 1);
						//powerPER_RTC(0, 0);
						powerPeripherals(0, 0);
						Serial.println(F("__tail_db == __head_db"));
						break;

					case 'V':	/* print battery voltage */
						Serial.print(F("Battery voltage: ")); Serial.println(readVcc() / 1000.0, 3 );
						break;

					case 'T':
						//powerPER_RTC(1,100);
						powerPeripherals(1, 100);
						Serial.print(F("Temperature (oC): ")); Serial.println( therm_getTemperature() *100 );
						//powerPER_RTC(0,0);
						powerPeripherals(0, 0);
						break;
						
					case 'H':
						//powerPER_RTC(1,100);
						powerPeripherals(1, 100);
						Serial.print(F("Humidity (%%): ")); Serial.println( therm_getHumidity() * 100 );
						//powerPER_RTC(0,0);
						powerPeripherals(0, 0);
						break;
						
					case 'C':
						//powerPER_RTC(1, 10);	//powerRTC(1, 10);
						powerPeripherals(1, 10);
						Serial.print(F("date time: ")); displayTime();
						//powerPER_RTC(0, 0);		//powerRTC(0, 1);
						powerPeripherals(0, 0);
						break;
					
					case 'G':
						switch( buf[1] ) {
							case '0': powerGPRSGPS( 0 ); break;
							case '1': powerGPRSGPS( 1 ); break;
						
							default:
								Serial.println(F("wrong G command!"));
						}
						break;
						
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
							case 'a':	/* module enable */
								switch( buf[2] ) {
									case '0': moduleAflags &= ~A_ENABLE; break;
									case '1': moduleAflags |= A_ENABLE; break;
									default:
										Serial.println(RCF( pWrongParameter ));
										break;
								}
								break;
							case 'l':	/* DNS lookup enable/disable */
								switch( buf[2] ) {
									case '0': moduleAflags &= ~A_DNSLOOKUP; break;
									case '1': moduleAflags |= A_DNSLOOKUP; break;
									default:
										Serial.println(RCF( pWrongParameter ));
										break;
								}
								break;
							default:
								Serial.println(RCF( pWrongParameter ));
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
  static datablock_t	db;


#define DEBUG_SLEEP_CYCLE

//	setupLoop();

	/* enter endless loop */
  	while(1) {
  		mySleep();


#if ENABLE_MAINTENANCE == 1
	{
//		int cnt32 = 10;
		
//  		Serial.println( F("out of sleep") );
//  		delay(2);
/*
			rx_interrupt = 0;
  		while(rx_interrupt || Serial.available() || (cnt32--)) {
  			if(rx_interrupt)
  				while(!Serial.available());
*/

			/* if rx_interrupt was set, then read corrupted character,
			 * wait for 500msec for second character and check if it was '+' */
			if(rx_interrupt) {
				Serial.read();
				delay(500);
				rx_interrupt = 0;
				
				if(Serial.available()) {

  				
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
			 * better timing */
			curSleepCycle = 0;
#endif
	
#ifdef DEBUG_SLEEP_CYCLE
#if 0
			Dp("curSleepCycle >= maxSleepCycle ");Dln(curSleepCycle);
#endif
#endif			
			db.nodeId = getNodeId();		//NODE_ID;
			db.batVolt = readVcc();
			
			powerPeripherals(1, 100);
			//powerPER_RTC(1, 100);	//powerRTC( 1, 10 );
//			displayTime();
			rtc_getTime(&dt);
			powerPeripherals(0, 0);
			//powerPER_RTC(0, 100);		//powerRTC( 0, 1 );
			
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

			    //powerRTC(1, 10);
			    powerPeripherals(1,50);
			    //powerPER_RTC(1, 250);
			    db.bhvTemp = therm_getTemperature() * 100;
			    db.bhvHumid = therm_getHumidity() * 100;
					db.bhvWeight = 75432;
								    
			    //powerPeripherals(0,0);
			    //powerRTC(0, 0);
//			    powerPER_RTC(0, 0);
			    doLog = false;
			    
					datetime2db( &dt, &db );

									
#define DEB_STUFF

#ifdef DEB_STUFF
          Serial.print( F(">>") );

		    	/* power voltage, before powering peripherals */
		    	Serial.print(db.batVolt); Serial.print( F(",") );
			
		    	/* temperature */
		    	Serial.print(db.bhvTemp); Serial.print( F(",") );
			
					/* humidity */
		    	Serial.print(db.bhvHumid); Serial.print( F("\n\r") );

		    	Serial.flush();
#endif


					/* put here code to store in EEPROM memory data blocks until
					 * net cycle is reached, and data are forwarded to internet server */

					db.entryType = ENTRY_DATA;


//					powerPER_RTC(1, 10);	//powerRTC(1, 10);
					if( mem_pushDatablock( &db ) ) {
						mem_stats();
//						Serial.println( F("pushed datablock to EEPROM successfully.") );
					} else {
						logError( erCANNOTPUSHDBINEEPROM );
						Serial.println( F("could not push dat datablock to EEPROM.") );
					}

					//powerPER_RTC(0, 0);		//powerRTC(0, 1);
					powerPeripherals(0, 0);
					
			}
  
			if(abs(curMinNetCycle - lastMinNetCycle) >= maxMinNetCycle)
				doNet = true;
				
			if(doNet)	{	//abs(curMinNetCycle - lastMinNetCycle) >= maxMinNetCycle) {
				/* maxMinNetCycle minutes have passed since last cycle
				 * do net-working stuff */
				unsigned long mil1=0, mil2=0;
				
#ifdef DEBUG_SLEEP_CYCLE
			    Dp("curMinNetCycle - lastMinNetCycle >= MaxMinNetCycle ");
			    D(curMinNetCycle); Dp(" "); Dln(lastMinNetCycle);
#endif

				if(!poweredGSM) {
					poweredGSM++;
//					Dp("Trying to power-up GSM/GPRS module - try "); Dln( poweredGSM );
					
					/* only apply power on the first time */
					if(poweredGSM == 1)
						powerGPRSGPS( 1 );
						mil1 = millis();
				}

#define MAX_POWEREDGSM	20

				while( poweredGSM < MAX_POWEREDGSM ) {
					if( !gsm_moduleInfo() ) {
//						Dp("GSM module is not ready yet! try ... "); Dln( poweredGSM );

						/* module is not yet ready, delay for some time (about 2 secs) */
						delay(500);
					} else break;	/* module ready! */
					
					poweredGSM++;
				}

				if(poweredGSM >= MAX_POWEREDGSM) {
					// module was not ready, so skip this session
					poweredGSM = 0;
					powerGPRSGPS( 0 );

					/* skipping session, we have 2 options, to try in the next
					 * open windows for NetCycle, or setup an earlier point,
					 * ie in 5 minutes, so we can try. But must set a counter,
					 * so this earlier point to be tries max ie 3 times, otherwise
					 * mark Net comms unavailable and try next NetCycle */ 
					lastMinNetCycle = curMinNetCycle;
					curSleepCycle = 0;	/* reset sleep cycle */

					continue; /* to the main while() loop */
				}
				
//#define CF( str )	(char *)( str )
//				Dln( poweredGSM );
				
//				Dlnp("GSM module is ready!");
				/* so module is ready */

//				Dlnp("Doing network stuff");

				/* disable local echo */
				gsm_sendcmdp( F("ATE0\r\n") );

//				if ( gsm_sendPin( CF( "7646") ) ) {		//1234" ) ) ) {
				if ( gsm_sendPin() ) {
					Dlnp("PIN set OK");
				} else {
					Dlnp("PIN was NOT set");
				}

				if(!gsm_sendrecvcmdtimeoutp( F(""), F("SMS Ready\r\n"), 20)) {
					Dlnp("module out of sync....");
				} else Dlnp("module in sync....");
				
				gsm_flushInput();
				
#define REG_TIMEOUT	15
		
				{
					uint8_t r;
					unsigned long mtimeout;
					
						mtimeout = millis() + REG_TIMEOUT * 1000;	/* timeout is 15 seconds */
						do {
							gsm_getRegistration( r );
							if((r == 1) || (r == 5))break;
							delay( 200 );
						} while( millis() < mtimeout );		/* set a timeout here, 15 secs */
				
						if( (r != 1) && (r != 5) ) { 
							poweredGSM = 0;
							powerGPRSGPS( 0 );


							/* reset NetCycle */
							lastMinNetCycle = curMinNetCycle;
							curSleepCycle = 0;	/* reset sleep cycle */

							doNet = 0;
							continue;
						}
				};
				
				
				gsm_initCIP();

#ifdef HTTP_API_GET
				do {
					uint8_t srvip[4];
						
						if( gsm_dnsLookup( srvip )) {
							Dlnp("successfully resolved domain name");
							memcpy(serverip, srvip, 4);
						} else {
							Dlnp("could not successfully resolve domain name, using old IP address");
						}
				} while(0);


/* disable wireless only if about to use GET methos,
 * for POST method leave wireless as is */
				gsm_doneCIP();
#endif

/* send data via a series of GET requests (time consuming, about 7 secs for each entry) */
#ifdef HTTP_API_GET

				if( gsm_activateBearerProfile() ) {
					if( http_initiateGetRequest() ) {
						datablock_t dd;

							//powerPER_RTC(1, 25);		//powerRTC(1, 100);
							powerPeripherals(1, 25);
							while(mem_popDatablock(&dd)) {
								if(!http_send_datablock_get( dd )) {
									logError( erCANNOTSENDDAT );
									Serial.print( RCF( pErrorCouldNot ) );
									Serial.println( RCF( pErrorSendDATblock ) );
//				  				Serial.println( F("error: could not send data block"));
				  				break;
								}
							}
							//powerPER_RTC(0, 0);		//powerRTC(0, 1);
							powerPeripherals(0, 0);
						
							do {
								uint16_t ii;
								uint8_t iii;
					
								dd.entryType = ENTRY_GSM;
								if( gsm_getBattery( ii ) ) {
//									Serial.print("Battery level: " ); Serial.println( ii );
									dd.gsmVolt = ii;
								}

								if( gsm_getSignalQuality( iii ) ) {
//									Serial.print("Signal quality: " ); Serial.println( iii );
									dd.gsmSig = iii;
								}
						
								dd.gsmPowerDur = millis() - mil1;
							
								if( http_send_datablock_get( dd ) ) {
								} else  {
									logError( erCANNOTSENDGSM );
									Serial.print( RCF( pErrorCouldNot ) );
									Serial.println( RCF( pErrorSendGSMblock ) );
									
									//Serial.println( F("error: could not send gsm block"));
								}
							} while(0);

							
							http_send_getconf_request();
							

							
					} else {
						logError( erCANNOTINITGETREQ );
						Serial.print( RCF( pErrorCouldNot ) );
						Serial.println( RCF( pErrorInitiateGet ) );
					}
				} else {
					logError( erCANNOTACTIVATEBEARER );
					Serial.print( RCF( pErrorCouldNot ) );
					Serial.println( RCF( pErrorActivateBearer ) );
				}

				http_terminateRequest();						
				gsm_deactivateBearerProfile();

	/* end of 	#if defined (HTTP_API_GET) */

#elif defined( HTTP_API_POST )
/* use POST request to send a series of data (faster...) */
				
				
				powerPeripherals(1, 25);
				
				/* wireless is already up by DNS request, save some
				 * time using this opportunity */
				
				if(gsm_initiateCIPRequest()) {
					/* ready to transmit POST data */
					if( http_send_post( mil1 ) ) {
						Serial.println( F("Succesfully send all data blocks with POST method!") );
					}

				} else {
					logError( erCANNOTINITPOST );
					Serial.print( RCF( pErrorCouldNot ) );
					Serial.println( RCF( pErrorInitiatePost ) );
				}
				
//				gsm_doneCIP();

				powerPeripherals(0, 0);
				
				/* do not need to actively terminate http_ since we are going to
				 * shutdown the module anyway! */

//				http_terminateRequest();						
//				gsm_deactivateBearerProfile();
				
#endif	/* HTTP_API_POST */
	
				powerGPRSGPS( 0 );
				poweredGSM = 0;
				mil2 = millis();
				
				Serial.print( F("GSM module was powered on for (msecs): "));
				Serial.println( mil2 - mil1 );
				
				lastMinNetCycle = curMinNetCycle;
				curSleepCycle = 0;	/* reset sleep cycle */
			}	/* in net cycle  */
		
		}	/* Sleep Cycle */
	
	}	/* main while logop  */
}
