/*
 * mem.h - memory module header
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-17
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __MEM_H__
#define __MEM_H__

#include "bms.h"
#include "rtc.h"

#define counter_type	uint16_t
//#define counter_type	uint32_t



#ifdef HAVE_DATA_H
/* if compiling with old data.h and data.cpp, then include
 * this file instead */
#include "fs2/data.h"
#endif

#ifndef HAVE_DATA_H


#if defined( LINUX_NATIVE_APP )

#define gpsCoordType	float

#elif defined( ARDUINO_ARCH_AVR )

#define gpsCoordType	float

#endif

#define ENTRY_DATA		0x1
#define ENTRY_GSM			0x2
#define ENTRY_GPS			0x3

#define ENTRY_ERROR		0x7

#define NODE_BITCOUNT		16
#define MAX_NODE_NUMBER	65535		/* depends on NODE_BITCOUNT */


typedef struct {
	uint16_t	nodeId: 16;		/* node identifier */

	struct {
		uint8_t	entryType:4;		/* entryType */
	
		uint8_t dayOfMonth: 5;
		uint8_t month: 4;
		uint8_t year: 7;
		uint8_t hour: 5;
		uint8_t minute: 6;
	};
	
//	newdatetime_t	dt;				/* date & time of date acquisition */

	union {
		struct {		/* ENTRY_DATA */
			uint16_t	batVolt;			/* node battery voltage */
			uint16_t	bhvTemp;			/* node temperature measurement */
			uint16_t	bhvHumid;			/* node humidity measurement */
			uint32_t	bhvWeight;		/* beehive weight measurement */
		};
		
		struct {		/* ENTRY_GSM */
			uint8_t		gsmSig;				/* GSM signal quality */
			uint16_t	gsmVolt;			/* GSM voltage */
			unsigned long int  gsmPowerDur;	/* number of milliseconds GSM module was powered on */
		};
		
		struct {		/* ENTRY_GPS */
			gpsCoordType gpsLon;		/* GPS longitude */
			gpsCoordType gpsLat;		/* GPS latitude */
		};
		struct {		/* ENTRY_ERROR */
			uint8_t dev;						/* device error, temp&humid error, GSM error, GPS error, RTC */
			uint32_t	dat;					/* error info */
		};			
	};	
} datablock_t;


#define BLOCK_SIZE	16


#endif	/* HAVE_DATA_H */



#ifdef __cplusplus
extern "C" {
#endif

extern counter_type	__head_db;		/* head pointer of datablocks */
extern counter_type	__tail_db;		/* tail pointer of datablocks */
extern counter_type	__max_db;			/* maximum data block available */

extern counter_type	__begin_addr;	/* first EEPROM address to access */
extern counter_type	__end_addr;		/* last EEPROM address to access */
extern counter_type __cnt_db;

void __ee_write(uint16_t addr, uint8_t data );
uint8_t __ee_read(uint16_t addr ); 
uint8_t __ee_readblock(uint16_t addr, char *data, uint8_t datalen);
void __ee_writeblock(uint16_t addr, char *data, uint8_t datalen);
 
void __ee_init(uint8_t addr, uint16_t dsize);
void __ee_end();

void mem_init(uint32_t dev_size, uint8_t dev_addr);
void mem_end();

void mem_stats();

bool mem_write(const void *p, uint8_t psize);
bool mem_read(const void *p, uint8_t psize);

bool mem_pushDatablock(datablock_t *db);

bool mem_popDatablock(datablock_t *db);
bool mem_readDatablocki(uint16_t index, datablock_t *db);
	
void datetime2db(datetime_t *dt, datablock_t *db);

void mem_readcounters();
void mem_storecounters();

void dumpDBrecord(datablock_t *db, int recno = -1);


#ifdef __cplusplus
}	/* extern "C" */
#endif

#endif	/* __MEM_H__ */
