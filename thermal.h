/*
 * thermal.h - temperature * humidity sensor header file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-16
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __THERMAL_H__
#define __THERMAL_H__

#define THERMAL_HTU21D	1

#ifdef THERMAL_HTU21D
extern "C" {
  #include <I2Cdev.h>
  #include <HTU21D.h>

  extern HTU21D therm;
  void therm_init();
  void therm_done();

  float	therm_getTemperature();
  float	therm_getHumidity();
};
#endif

#endif	/* __THERMAL_H__ */
