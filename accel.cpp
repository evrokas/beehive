/*
 * accel.cpp - Accelerator functions file
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
#include <math.h>

#include "utils.h"
#include "accel.h"

#ifdef	ACCEL_ADXL345

 #include <I2Cdev.h>
 #include <ADXL345.h>

 ADXL345 accel;


#endif	/* ACCEL_ADXL345 */


void accel_init()
{
	accel.initialize();
	if(!accel.testConnection()) {
		Dln("Could not connect to acceleration sensor device.");
	}

	accel.setActivityAC( true );
	accel.setRate( ADXL345_RATE_100 );
	accel.setRange( ADXL345_RANGE_4G );


	accel.setActivityThreshold( 15 );
	accel.setInactivityThreshold( 15 );
	accel.setInactivityTime( 1 );
	accel.setActivityXEnabled( true );
	accel.setActivityYEnabled( true );
	accel.setActivityZEnabled( true );
	
	accel.setInactivityXEnabled( true );
	accel.setInactivityYEnabled( true );
	accel.setInactivityZEnabled( true );
	
	accel.setIntActivityEnabled( true );
	accel.setIntActivityPin( 0 );
	
	accel.setLinkEnabled( true );
	accel.setMeasureEnabled( true );
	
}

char accel_getactivitysource()
{
  return (accel.getIntActivitySource());
}

void accel_getxyz(int16_t *x, int16_t *y, int16_t *z)
{
/*        void getAcceleration(int16_t* x, int16_t* y, int16_t* z);*/

	accel.getAcceleration(x, y, z);
}

void accel_calculaterollpitch(int16_t x, int16_t y, int16_t z, float *roll, float *pitch)
{
	*roll = atan2( y, z ) * 57.3;
	*pitch = atan2( (-x), sqrt( y * y + z * z)) * 57.3;
}
