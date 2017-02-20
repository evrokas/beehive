/*
 * accel.h - Accelerator header file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-16
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __ACCEL_H__
#define __ACCEL_H__

/* define type of accelerator chip */
#define ACCEL_ADXL345	1


extern "C" {

void accel_init();
void accel_getxyz(int16_t *x, int16_t *y, int16_t *z);
void accel_calculaterollpitch(int16_t x, int16_t y, int16_t z, float *roll, float *pitch);

char accel_getactivitysource();

};


#endif	/* __ACCEL_H__ */
