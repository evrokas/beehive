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
//#include "data.h"


#define counter_type	uint16_t
//#define counter_type	uint32_t


#define BLOCK_SIZE	32

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


typedef struct {
	uint16_t	nodeId;
	uint16_t	batVolt;
	uint16_t	bhvTemp;
	uint16_t	bhvHumid;
	uint32_t	rtcDateTime;

#if 1
	uint8_t		gsmSig;
	uint16_t	gsmVolt;
#endif

#if 1
	gpsCoordType gpsLon;
	gpsCoordType gpsLat;	
#endif

	uint32_t		bhvWeight;


#if 1
	datetime_t	dt;
#endif
} datablock_t;


#endif	/* HAVE_DATA_H */



#ifdef __cplusplus
extern "C" {
#endif

extern counter_type	__head_db;		/* head pointer of datablocks */
extern counter_type	__tail_db;		/* tail pointer of datablocks */
extern counter_type	__max_db;			/* maximum data block available */

extern counter_type	__begin_addr;	/* first EEPROM address to access */
extern counter_type	__end_addr;		/* last EEPROM address to access */

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
	



#ifdef __cplusplus
}	/* extern "C" */
#endif

#endif	/* __MEM_H__ */
