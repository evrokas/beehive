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
#include "Wire.h"

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
	DDRC |=		B00000001;	/* peripheral control (bit 0, 1 = on, 0 = off) */
	PORTC &=	B11111111;	/* last -1- should be 1 in order for peripherals to be off */

	DDRB |=		B11000000;	/* GSM and RTC control (bit 7 (1 = off, 0 = on) and bit 6 (1 = on, 0 = off respectively) */
	PORTB &=	B10111111;	/* turn GSM off, RTC off */
}
#if 0
void powerPeripherals(unsigned char onoff, unsigned char mdel)
{
	switch(onoff) {
		case 0: { /* power off */
			//__sbi(PORTC, 0);

			Dnln(2, "Per = on");
			PORTC |= B00000001;

			delayMicroseconds(50);
#if TURNOFF_I2C_PULLUPS_DURING_SLEEP == 1
			twi_disable_pullups();
#endif

			break;
		}
		
		case 1: { /* power on */
			//__cbi(PORTC, 0);

#if TURNOFF_I2C_PULLUPS_DURING_SLEEP == 1
			twi_enable_pullups();
#endif

			delayMicroseconds(50);
			PORTC &= B11111110;
			
			Dnln(2, "Per = off");

			break;
		}
	}
	
	delay( mdel );	/* delay for specified number of msecs */
}
#endif


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

/* combined power on/off of peripherals and RTC */
void powerPeripherals(unsigned char onoff, unsigned char mdel)
{
	switch(onoff) {
		case 0: { /* power off */
			//__sbi(PORTC, 0);

			Dnln(2, "Per = off");
			PORTC |= B00000001;
			PORTB &= B10111111;

#if TURNOFF_I2C_PULLUPS_DURING_SLEEP == 1
			delayMicroseconds(50);
			twi_disable_pullups();
#endif

			break;
		}
		
		case 1: { /* power on */
			//__cbi(PORTC, 0);

#if TURNOFF_I2C_PULLUPS_DURING_SLEEP == 1
			twi_enable_pullups();
			delayMicroseconds(50);
#endif

			PORTC &= B11111110;
			PORTB |= B01000000;
			
			Dnln(2, "Per = on");

#if 1
			switch( i2c_clearBus() ) {
				case 0: Wire.begin(); break;
				case 1: Serial.println(F("I2C bus returned: 1")); break;
				case 2: Serial.println(F("I2C bus returned: 2")); break;
				case 3: Serial.println(F("I2C bus returned: 3")); break;
			}
#endif


			break;
		}
	}
	
	delay( mdel );	/* delay for specified number of msecs */

//	powerPeripherals(onoff, 0);
//	powerRTC(onoff, mdel);
}

		

float	VREF = 1.035;		/* 1.035 * 1100 = 1138.55 */
uint32_t InternalReferenceVoltage = (uint32_t)(1100);


/* read Vcc using internal 1.1V reference */
unsigned long readVcc()
{
  //float f;
//  unsigned long f;
  
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
//    f = ( ( ( VREF * InternalReferenceVoltage * 1024.0 ) / result) + 5.0);	// / 10.0;
	result = ( (VREF * 1125300) / result) + 5.0;
//  result = f;
  return result; // Vcc in millivolts
}

void setVccFactor(float avref)
{
	VREF = avref;
}

float getVccFactor()
{
	return (VREF);
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

void eepromSetLong( uint16_t aaddr, uint32_t dat )
{
  eepromSetWord( aaddr, (dat & 0xffff) );
  eepromSetWord( aaddr+2, (dat >> 16) & 0xffff );
}


float eepromGetFloat( uint16_t aaddr )
{
	union {
		float f;
		long l;
	} f_l;
	f_l.l = eepromGetLong( aaddr );
	
	return (f_l.f);
}

void eepromSetFloat( uint16_t aaddr, float dat )
{
	union {
		float f;
		long l;
	} f_l;

	f_l.f = dat;
	
  eepromSetLong( aaddr, f_l.l );
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
  int aa;
  
	for(aa=0;aa<acnt;aa++) {
		*dat++ = eepromGetChar( aaddr + aa );
	}
}
		
void eepromSetStr(uint16_t aaddr, int acnt, char *dat)
{
  int aa;

	for(aa=0;aa<acnt;aa++) {
  	eepromSetChar( aaddr + aa, *dat++);
	}
}




/**
 * This routine turns off the I2C bus and clears it
 * on return SCA and SCL pins are tri-state inputs.
 * You need to call Wire.begin() after this to re-enable I2C
 * This routine does NOT use the Wire library at all.
 *
 * returns 0 if bus cleared
 *         1 if SCL held low.
 *         2 if SDA held low by slave clock stretch for > 2sec
 *         3 if SDA held low after 20 clocks.
 */
uint8_t i2c_clearBus() {
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

//  delay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master. 
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  if(SDA_LOW)Serial.println(F("I2C SDA Low") );

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
  // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(SCL, INPUT); // release SCL LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(SDA, INPUT); // remove pullup.
  pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(SDA, INPUT); // remove output low
  pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(SCL, INPUT);
  return 0; // all ok
}



uint16_t getStackPointer()
{
  union {
  	uint8_t c[2];
  	uint16_t i;
	} t;
	
	t.c[0] = SPL;
	t.c[1] = SPH;
	
	return (t.i);
}
