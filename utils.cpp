/*
 * utils.c - utility functions file
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
//#include <EEPROM.h>

#include "bms.h"
#include "utils.h"
#include <avr/eeprom.h>



#if TURNOFF_I2C_PULLUPS_DURING_SLEEP == 1
unsigned char __pullup_sda=0;
unsigned char __pullup_scl=0;
#endif


void twi_disable_pullups()
{
#if TURNOFF_I2C_PULLUPS_DURING_SLEEP == 1
	__pullup_sda = SDA;
	__pullup_scl = SCL;
	
	digitalWrite(SDA, 0);
	digitalWrite(SCL, 0);
	Dnln(2, "Pullup off");
#endif
}


void twi_enable_pullups()
{
#if TURNOFF_I2C_PULLUPS_DURING_SLEEP == 1
  digitalWrite(SDA, __pullup_sda);
  digitalWrite(SCL, __pullup_scl);
  Dnln(2, "Pullup on");
#endif
}


void setupPeripheralsControl()
{
	DDRC |=		B00000001;
	PORTC &=	B11111111;	/* last -1- should be 1 in order for peripherals to be off */

	DDRB |=		B11000000;
	PORTB &=	B10111111;
}


void powerPeripherals(unsigned char onoff, unsigned char mdel)
{
	switch(onoff) {
		case 0: { /* power off */
			//__sbi(PORTC, 0);

#if TURNOFF_I2C_PULLUPS_DURING_SLEEP == 1
			twi_disable_pullups();
#endif
			Dnln(2, "Per = on");
			PORTC |= B00000001;
			break;
		}
		
		case 1: { /* power on */
			//__cbi(PORTC, 0);
			PORTC &= B11111110;
			
			Dnln(2, "Per = off");
#if TURNOFF_I2C_PULLUPS_DURING_SLEEP == 1
			twi_enable_pullups();
#endif

			break;
		}
	}
	
	delay( mdel );	/* delay for specified number of msecs */
}

void powerGPRSGPS(unsigned char onoff)
{
	switch(onoff) {
		case 0:  	/* power off */
			PORTB |= B10000000;
			break;
		case 1: 	/* power on */
			PORTB &= B01111111;
			break;
	}
}


/* power real-time clock on/off */
void powerRTC(unsigned char onoff, unsigned char mdel)
{
	switch(onoff) {
		case 0: 	/* power off */
			PORTB &= B10111111;
			break;
		case 1:   /* power on */
			PORTB |= B01000000;
			break;
	}
	delay( mdel );
}

float	VREF = 1.035;
uint32_t InternalReferenceVoltage = (uint32_t)(1100);


/* read Vcc using internal 1.1V reference */
unsigned long readVcc()
{
  float f;
  
    ADCSRA |= (1 << ADEN);

	// Read 1.1V reference against AVcc
	// set the reference to Vcc and the measurement to the internal 1.1V reference
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA,ADSC)); // measuring

	uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
	uint8_t high = ADCH; // unlocks both

	ADCSRA ^= (1 << ADEN);
	
	unsigned long result = (high<<8) | low;
  
//  result = 1125300L * (4.96/5.5) / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
//  result = 1125300L * VCC_CORRECTION / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*10
    f = ( ( ( VREF * InternalReferenceVoltage * 1024.0 ) / result) + 5.0);	// / 10.0;
  result = f;
  return result; // Vcc in millivolts
}

void setVccFactor(float avref)
{
	VREF = avref;
}


#define EEPROM_SIZE	1024

uint16_t eepromLastAddress = 0;

void eepromMemBase( uint16_t amembase )
{
	if(amembase < EEPROM_SIZE)
		eepromLastAddress = amembase;
}

uint16_t eepromGetAddr( uint8_t asize )
{
  uint16_t nsize = eepromLastAddress;

  	eepromLastAddress += asize;
  	
	return (nsize);
}

uint16_t eepromGetLastAddr()
{
  return (eepromLastAddress);
}


uint8_t	eepromGetByte( uint16_t aaddr )
{
//  return ( EEPROM[ aaddr ] );
  return ( eeprom_read_byte( (uint8_t *)aaddr ) );
}

void eepromSetByte( uint16_t aaddr, uint8_t dat )
{
//	EEPROM[ aaddr ] = dat;
	eeprom_write_byte( (uint8_t *)aaddr, dat );
}

uint16_t eepromGetWord( uint16_t aaddr )
{
	return (eepromGetByte(  aaddr ) + ((uint16_t)eepromGetByte( aaddr + 1 ) << 8) );
}

void eepromSetWord( uint16_t aaddr, uint16_t dat )
{
	eepromSetByte(aaddr, dat & 0xff );
	eepromSetByte(aaddr+1, (dat >> 8) & 0xff );
}


uint32_t eepromGetLong( uint16_t aaddr )
{
	return (eepromGetWord( aaddr ) + ((uint32_t)(eepromGetWord( aaddr + 2 )) << 16) );
}

float eepromGetFloat( uint16_t aaddr )
{
  return (float)(eepromGetLong( aaddr ) );
}

char eepromGetChar( uint16_t aaddr )
{
  return (char)(eepromGetByte( aaddr ) );
}

void eepromSetChar( uint16_t aaddr, char dat)
{
	eepromSetByte( aaddr, dat );
}

void eepromGetStr(uint16_t aaddr, int acnt, char *dat)
{
  int aa=0;
  
	while(acnt >= 0) {
		*dat = eepromGetChar( aaddr + aa );
		dat++; acnt--; aa++;
	}
}
		
void eepromSetStr(uint16_t aaddr, int acnt, char *dat)
{
  int aa=0;

  while(acnt >= 0) {
  	eepromSetChar( aaddr + aa, *dat);
  	aa++; dat++; acnt--;
	}
}
