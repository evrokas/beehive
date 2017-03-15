/*
 * data.h - data header file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-17
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __DATA_H__
#define __DATA_H__


#include <stdint.h>
#include "bms.h"


// add debugging info
#define	RECORD_DEBUG	1

// add variables for statistics in record
#define RECORD_STATS	1


// define the follwing if we have the weight sensor and want to log beehive weight
//#define	HAVE_WEIGHT_SENSOR	1



#define	p_nodeId	0
//#define p_mcuTemp	1
#define p_batVolt	2
#define p_bhvTemp	3
#define p_bhvHumid	4
#define p_rtcDateTime	5
#define p_gsmSig	6
#define p_gsmVolt	7
#define p_gpsLong	8
#define p_gpsLat	9
#define p_bhvWeight	10

#if RECORD_STATS == 1

	#define p_EntryWritten	11	// many times a record is written

#endif	/* RECORD_STATS */



typedef struct {
	uint8_t	nibbleStart, nibbleStop;
} nibblepos_t;

#if 0
nibblepos_t	block[] = {
	{ 0, 3},		// nodeId
	{ 4, 5},		// mcuTemp
	{ 6, 9},		// batVolt
	{10,11},		// bhvTemp
	{12,13},		// bhvHumid
	{14,20},		// rtcDateTime
	{21,22},		// gsmSig
	{23,26},		// gsmVolt
	{27,35},		// gpsLong
	{36,44},		// gpsLat
	{45,50},		// bhvWeight
};
#endif

#define SECTOR_HEADER_SIZE	4
#define SECTOR_PAYLOAD_SIZE	28


#define SECTOR_TYPE_COUNT	4

#define SECTOR_DATA		0
#define SECTOR_CONFIG	1
#define SECTOR_unused	2
#define SECTOR_BAD		3
/* if more sector types are needed, then we can extend st? bits */


typedef struct {
	uint8_t header[SECTOR_HEADER_SIZE];		// 2 bits of sector type, 13 bits of head pointer, 13 bits of tail pointer (sum: 28bits) */
/*	+[3]-+----+----+----+----+----+----+----+
 *  |st1 |st0 |    |    |    |    |tl12|tl11|
 *	+----+----+----+----+----+----+----+----+
 *	+[2]-+----+----+----+----+----+----+----+
 *  |tl10|tl9 |tl8 |tl7 |tl6 |tl5 |tl4 |tl3 |
 *	+----+----+----+----+----+----+----+----+
 *	+[1]-+----+----+----+----+----+----+----+
 *  |tl2 |tl1 |tl0 |hd12|hd11|hd10|hd9 |hd8 |
 *	+----+----+----+----+----+----+----+----+
 *	+[0]-+----+----+----+----+----+----+----+
 *  |hd7 |hd6 |hd5 |hd4 |hd3 |hd2 |hd1 |hd0 |
 *	+----+----+----+----+----+----+----+----+
 */

 uint8_t payload[SECTOR_PAYLOAD_SIZE];
} SECTOR;


typedef struct {
	uint16_t rdCount;
	uint16_t wrCount;
} STATS;


#define DECODE_HEAD(xx)	((xx[0]&0xff)|(((uint16_t)(xx[1]&0x1f))<<8))
#define DECODE_TAIL(xx)	((xx[1]>>5)|(((uint16_t)xx[2])<<3)|((uint16_t)(xx[3]&0x03)<<11))
#define DECODE_TYPE(xx)	(xx[3]>>6)

#define ENCODE_HEAD(xx,head)	(xx[0]=(head&0xff));(xx[1]=(xx[1]&0xe0)|((head&0x1f00)>>8))
#define ENCODE_TAIL(xx,tail)	(xx[1]=(xx[1]&0x1f)|((tail&0x07)<<5));(xx[2]=(tail>>3&0xff));(xx[3]=(xx[3]&0xc0)|((tail>>11)&0x03))
#define ENCODE_TYPE(xx,type)	(xx[3]=(xx[3]&0x3f)|((type&0x3)<<6));

#if defined( LINUX_NATIVE_APP )

#define gpsCoordType	float

#elif defined( ARDUINO_ARCH_AVR )

#define gpsCoordType	float

#endif


typedef struct {
	uint16_t	nodeId;
//	uint8_t		mcuTemp;
	uint16_t	batVolt;
	uint16_t	bhvTemp;
	uint16_t	bhvHumid;
	uint32_t	rtcDateTime;
	uint8_t		gsmSig;
	uint16_t	gsmVolt;

	gpsCoordType gpsLon;
	gpsCoordType gpsLat;	

#if HAVE_WEIGHT_SENSOR == 1
	uint32_t		bhWeight[3];
#endif

} datablock_t;


typedef struct {
	uint16_t HEAD;
	uint16_t TAIL;
} RING;





#ifdef __cplusplus
extern "C" {
#endif


void filesystem_init();
void filesystem_done();
void __fs_readSector(uint16_t, void *);
void __fs_writeSector(uint16_t, void *);

void fs_writeSector(SECTOR *);

void dumpStats();
	
#ifdef __cplusplus
};
#endif


#endif	/* __DATA_H__ */
