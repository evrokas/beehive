/*
 * utils.h - mutility functions header file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-16
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <Arduino.h>
#include "bms.h"

#ifndef cbi
#define __cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define __sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define PeripheralPort	PORTC
#define PeripheralPin		0

#define GSMPort					PORTB
#define GSMBit					7

#define RTCPort					PORTV
#define RTCBit					6




#define DEBUG_ACCEL	1

#define DEBUG_PRINT	1

/* DEBUG_LEVEL this specifies the level of info printed on debug
 * output. Levels are as follows:
 *  1 - simple debugging messages
 *  2 - i/o messages important for showing program flow
 *  3 - critical i/o messages
 */

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL	1
#endif

#define DEB_LEV1	1
#define DEB_LEV2	2
#define DEB_LEV3	3



#ifdef DEBUG_PRINT
  #define DBAUD		9600
  #define Dinit		Serial.begin( DBAUD )
  #define D(dstr)	Serial.print(dstr)
  #define Dln(dstr)	Serial.println(dstr)
  #define Dp(dstr)	Serial.print( F(dstr) )
  #define Dlnp(dstr)	Serial.println( F(dstr) );
#if 1
  #define Dn(dlev,dstr)		if(dlev<=DEBUG_LEVEL){ D(dstr); }
  #define Dnln(dlev,dstr)	if(dlev<=DEBUG_LEVEL){ Dln(dstr); }
#endif

#else
  #define DBAUD
  #define Dinit		;
  #define D(dstr)	;
  #define Dln(dstr)	;
  #define Dp(dstr)	;
  #define Dlnp(dstr)	;

#if 1
  #define Dn(dlev,dstr)	;
  #define Dnln(dlev,dstr)	;
#endif


#endif /* DEBUG_PRINT */



#define APIKEY_SIZE		8
#define APN_SIZE			24
#define USER_SIZE			8
#define PASS_SIZE			8
#define URL_SIZE			24
#define SIMPIN_SIZE			4
#define SIMICCID_SIZE		20
#define LOGPROG_SIZE  12
#define NETPROG_SIZE  12

#define EEPROM_MAXSTR	24		/* set to maximum length of the above */

#define E_URL				1
#define E_APN				2
#define E_USER			3
#define E_PASS			4
#define E_APIKEY		5
#define E_SIMPIN		6
#define E_SIMICCID	7
#define E_LOGPROG 	8
#define E_NETPROG 	9

extern uint8_t	addrNodeId;
extern uint8_t	addrAPIKEY;
extern uint8_t	addrAPN;
extern uint8_t	addrUSER;
extern uint8_t	addrPASS;
extern uint8_t	addrURL;
extern uint8_t	addrPORT;
extern uint8_t addrLogCycle;
extern uint8_t addrNetCycle;
extern uint8_t addrSIMPIN;
extern uint8_t addrSIMICCID;
extern uint8_t addrVCCfactor;
extern uint8_t addrAflags;
extern uint8_t addrLogProg;
extern uint8_t addrNetProg;


#ifdef __cplusplus
extern "C" {
#endif

/* disable I2C internal pullups */
void twi_disable_pullups();

/* enable I2C internal pullups */
void twi_enable_pullups();

/* setup control bits for peripheral control */
void setupPeripheralsControl(void);

/* function to power on/off peripherals */
void powerPeripherals(unsigned char onoff, unsigned char mdel);

/* function to power on/off GSM/GPRS and GPS modules */
void powerGPRSGPS(unsigned char onoff);

/* function to power RTC on/off */
void powerRTC(unsigned char onoff, unsigned char mdel);

/* function to combine power on/off of periperals and RTC */
//void powerPER_RTC(unsigned char onoff, unsigned char mdel);

/* function to read power line voltage */
unsigned long readVcc();

void setVccFactor(float avref);
float getVccFactor();

void eepromMemBase( uint16_t amembase );
uint16_t eepromGetLastAddr();

uint16_t eepromGetAddr( uint8_t asize );
uint8_t	eepromGetByte( uint16_t aaddr );
void eepromSetByte( uint16_t aaddr, uint8_t dat );
uint16_t eepromGetWord( uint16_t aaddr );
void eepromSetWord( uint16_t aaddr, uint16_t aword );
uint32_t eepromGetLong( uint16_t aaddr );
void eepromSetLong( uint16_t aaddr, uint32_t dat );
float eepromGetFloat( uint16_t aaddr );
void eepromSetFloat( uint16_t aaddr, float dat );
char eepromGetChar( uint16_t aaddr );
void eepromSetChar( uint16_t aaddr, char dat );

void eepromGetStr(uint16_t aaddr, int acnt, char *dat);
void eepromSetStr(uint16_t aaddr, int acnt, char *dat);
bool transmitEEPROMstr(uint8_t ecode, Stream &strm, bool debugSerial = false);
bool transmitEEPROMstrd(uint8_t ecode, Stream &strm);

void initializeEEPROM();

/* read Node number from EEPROM */
uint16_t getNodeId();

/* write Node number to EEPROM */
void setNodeId(uint16_t nodeid);

/* read <ecode> from EEPROM */
char *getEEPROMstr(uint8_t ecode, char *dat);

/* write <ecode> to EEPROM */
char *setEEPROMstr(uint8_t ecode, char *dat);

/* write LogCycle to EEPROM */
void setLogCycle(uint8_t dat);

/* read LogCycle from EEPROM */
uint8_t getLogCycle();

/* write NetCycle to EEPROM */
void setNetCycle(uint8_t dat);

/* read NetCycle from EEPROM */
uint8_t getNetCycle();

/* read Server port from EEPROM */
uint16_t getServerPort();

/* write Seriver port to EEPROM */
void setServerPort( uint16_t dat );

/* write VCC factor to EEPROM */
void setVCC( float dat );

/* read VCC factor from EEPROM */
float getVCC();

/* read Aflags from EEPROM */
uint16_t getAflags();

/* write Aflags to EEPROM */
void setAflags(uint16_t dat);


void loadVariablesFromEEPROM();

uint8_t i2c_clearBus();

uint8_t getNetCycle();
void setNetCycle(uint8_t dat);
void setLogCycle(uint8_t dat);
uint8_t getLogCycle();

extern uint16_t maxMinLogCycle;
extern uint16_t maxMinNetCycle;

uint16_t getStackPointer();

#ifdef __cplusplus
} /* extern */
#endif


#endif /* __UTILS_H__ */
