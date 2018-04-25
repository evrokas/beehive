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

#define EEPROM_MAXSTR	24		/* set to maximum length of the above */

#define E_URL		1
#define E_APN		2
#define E_USER	3
#define E_PASS	4
#define E_APIKEY		5
#define E_SIMPIN		6
#define E_SIMICCID	7




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
bool transmitEEPROMstr(uint8_t ecode, Stream &strm);


void initializeEEPROM();
uint16_t getNodeId();
void setNodeId(uint16_t nodeid);
char *getEEPROMstr(uint8_t ecode, char *dat);
char *setEEPROMstr(uint8_t ecode, char *dat);
void setLogCycle(uint8_t dat);
uint8_t getLogCycle();
void setNetCycle(uint8_t dat);
uint8_t getNetCycle();
uint16_t getServerPort();
void setServerPort( uint16_t dat );
void setVCC( float dat );
float getVCC();
void loadVariablesFromEEPROM();



#ifdef __cplusplus
} /* extern */
#endif


#endif /* __UTILS_H__ */
