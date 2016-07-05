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
}
