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

#include <Arduino.h>
#include <avr/eeprom.h>

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
#define HAVE_ACCEL	1

/* set to 1 if project has nRF24L01 RF module */
#define HAVE_RF		1



/* pin which is used to power peripherals on/off */
#define PERIPH_POWER_PIN	PC0


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
#define CYCLES_SLEEP_COUNT	6

/* daily data logging frequency */
#define DAILY_LOG_FREQ	(5 * 24)	// 5 times/hours, every 12 minutes

/* daily network communication frequency */
#define DAILY_NET_FREQ	(12)		// 12 times per day, every 2 hours


/* daily data logging period */
#define DAILY_LOG_PERIOD	(1440 / DAILY_LOG_FREQ)

/* daily network communication period */
#define DAILY_NET_PERIOD	(1440 / DAILY_NET_FREQ)


/* number of cycles for data logging */
#define CYCLES_LOG_COUNT	((86400 / DAILY_LOG_FREQ) / SLEEP_CYCLE)


/* nuimber of cycles for network data communication */
#define CYCLES_NET_COUNT	((86400 / DAILY_NET_FREQ) / SLEEP_CYCLE)


#if 0
unsigned short int EEMEM MaxLogCycle = CYCLES_LOG_COUNT;
unsigned short int EEMEM MaxNetCycle = CYCLES_NET_COUNT;
#endif


/* this is the VCC correction variable used in readVcc() */
#define VCC_CORRECTION  		1	//0.9854
#define InternalReferenceVoltage	(1062.0)


#endif	/* __BMS_H__ */
