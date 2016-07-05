/*
 * thermal.cpp - temperature & humidity sensor functions file
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
#include "thermal.h"

#ifdef	THERMAL_HTU21D

 #include <I2Cdev.h>
 #include <HTU21D.h>

HTU21D	therm;

 
#endif	/* THERMAL_HTU21D */

void therm_init()
{
	therm.initialize();

	if(!therm.testConnection()) {
		Dln("Could not connect to temperature sensor.");
	}
}

void therm_done()
{
}

float	therm_getTemperature()
{
	return ( therm.getTemperature() );
}

float	therm_getHumidity()
{
  return ( therm.getHumidity() );
}

