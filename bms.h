/*
 * bms.h - main project header file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-16
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __BMS_H__
#define __BMS_H__


#if !defined( LINUX_NATIVE_APP )

#include <Arduino.h>
#include <avr/eeprom.h>
#include <WString.h>

#endif

#include <stdint.h>
#include "ver.h"

/* system board revision string */
#define BOARD_REVISION		"3"
#define BOARD_REV_MAX			3
#define BOARD_REV_MIN			0

/* firmware revision number */
#ifndef FIRMWARE_REVISION
#define FIRMWARE_REVISION	"alpha"
#endif
#define FIRMWARE_REV_MAX	0
#define FIRMWARE_REV_MIN	0


/* setup defines */

//#define LP_SLEEP_MODE	SLEEP_2S		/* 8 seconds sleep */
#define LP_SLEEP_MODE	SLEEP_8S		/* 8 seconds sleep */

/* set to 1 if project has Ublox Neo6M GPS receiver */
#define	HAVE_GPS	1

/* set to 1 if project has SIM800L GSM/GPRS module */
#define HAVE_GSM_GPRS	1

/* set to 1 if project has temperature/humidity sensor */
#define HAVE_THSENSOR	1

/* set to 1 if project has ADXL345 accelerator */
#define HAVE_ACCEL	0

/* set to 1 if project has nRF24L01 RF module */
#define HAVE_RF		0

/* set to 1 if project has HX711 weight sensor module */
#define HAVE_WEIGHT_SENSOR	0


/* pin which is used to power peripherals on/off */
#define PERIPH_POWER_PIN	PC0


/* set this to 1 to enable maintenance mode, otherwise set to 0 */
#define ENABLE_MAINTENANCE	0

/* set to 1 if storage of data is enabled, set to 0 otherwise */
#define ENABLE_DATAPUSHING	11



/* this factor is determined by trial and error, may be different
 * for various chips, may have to remove hard coding and replace it
 * in the EEPROM */
 
#define VCC_CORRECTION_FACTOR	0.9854

/* pins for software serial for GPS communication */
#define GPS_RX_PIN	6
#define GPS_TX_PIN	7

/* pins for software serial for GSM/GPRS communication */
#define GSM_RX_PIN	9
#define GSM_TX_PIN	10


/* set this to 1 if you want to shut off pullups on I2C
 * bus during sleep. (This prevents current leaking through
 * pullup resistors to the modules of the I2C bus. This could be
 * also prevented if one removes all pullup resistors from all
 * modules on the I2C bus, but turning off pullup is much easier */
#define TURNOFF_I2C_PULLUPS_DURING_SLEEP	1



/* Timinig constants */

/* seconds that wdt timeout cycle lasts */
#define SLEEP_CYCLE	8

/* number of sleep cycles to test for time */
//#define CYCLES_SLEEP_COUNT	2
#define CYCLES_SLEEP_COUNT	7


#define VALUE_TO_STRING(x)	#x
#define VALUE(x)		VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var)	#var "=" VALUE(var)

/* SPEED_RATIO */
 
//#define SPEED_RATIO	24		/* cycles 15 */
//#define SPEED_RATIO	48		/* cycles 7 */
//#define SPEED_RATIO	96		/* cycles 3 */
#define SPEED_RATIO	1

/* daily data logging frequency */
#define DAILY_LOG_FREQ	(24 * 5 *6 * (SPEED_RATIO))	// 24 hours, 5 times per hour
//#define DAILY_LOG_FREQ	(24 * 5 * 2 * 2 * 2*(SPEED_RATIO))	// 24 hours, 5 times per hour

/* daily network communication frequency */
//#define DAILY_NET_FREQ	(24 * 1 * (SPEED_RATIO))	// 24 hours, 1 time per hour
#define DAILY_NET_FREQ	(24  * 2 * (SPEED_RATIO))	// 24 hours, 2 time per hour
//#define DAILY_NET_FREQ	(24 * 5 * 2 * 2 * (SPEED_RATIO))	// 24 hours, 1 time per hour

//#pragma message(VAR_NAME_VALUE( DAILY_LOG_FREQ) )


/* daily data logging period */
#define DAILY_LOG_PERIOD	(1440 / DAILY_LOG_FREQ)

/* daily network communication period */
#define DAILY_NET_PERIOD	(1440 / DAILY_NET_FREQ)


//#define InternalReferenceVoltage	(1062.0)
//#define InternalReferenceVoltage	(1100.0)
extern uint32_t InternalReferenceVoltage;	// = (uint32_t)(1100);

#endif	/* __BMS_H__ */
