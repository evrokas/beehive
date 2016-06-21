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
#include "bms.h"
#include "utils.h"



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
	DDRC |= B00000001;
	PORTC &= B11111110;
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

void powerGPRSGPS(unsigned char onoff, unsigned char mdel)
{
	switch(onoff) {
		case 0: { 	/* power off */
			}; break;
		case 1: {	/* power on */
			}; break;
	}
}


/* power real-time clock on/off */
void powerRTC(unsigned char onoff, unsigned char mdel)
{
	switch(onoff) {
		case 0: {	/* power off */
			}; break;
		case 1: { /* power on */
			}; break;
	}
	
	delay( mdel );
}



/* read Vcc using internal 1.1V reference */
unsigned long readVcc()
{
	// Read 1.1V reference against AVcc
	// set the reference to Vcc and the measurement to the internal 1.1V reference
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

	delay(1); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA,ADSC)); // measuring

	uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
	uint8_t high = ADCH; // unlocks both

	unsigned long result = (high<<8) | low;
  
//  result = 1125300L * (4.96/5.5) / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  result = 1125300L * VCC_CORRECTION / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*10
//  result = ( ( ( (unsigned long)InternalReferenceVoltage * 1024 ) / result) + 5) / 10;
//  return result; // Vcc in millivolts
}
