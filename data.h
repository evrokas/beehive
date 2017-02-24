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

#define	p_nodeId	0
#define p_mcuTemp	4
#define p_batVolt	7
#define p_bhvTemp	11
#define p_bhvHumid	15
#define p_rtcDate	19
#define p_rtcTime	25
#define p_gsmSig	31
#define p_gsmVolt	33
#define p_gpsLong	37
#define p_gpsLat	46
#define p_bhvWeight	55	


typedef struct {
	uint8_t	nibbleStart, nibbleStop;
} nibblepos_t;

nibblepos_t	block[] = {
	{ 0, 3},		// nodeId
	{ 4, 6},		// mcuTemp
	{ 7,10},		// batVolt
	{11,14},		// bhvTemp
	{15,18},		// bhvHumid
	{19,24},		// rtcDate
	{25,30},		// rtcTime
	{31,32},		// gsmSig
	{33,36},		// gsmVolt
	{37,45},		// gpsLong
	{46,54},		// gpsLat
	{55,59},		// bhvWeight

typedef struct {
	uint16_t	nodeId;
	uint8_t		mcuTemp;
	uint16_t	batVolt;
	uint16_t	bhvTemp;
	uint16_t	bhvHumid;
	uint8_t		rtcDate[3];		/* 3 bytes, 6 nibbles, first 2 nibbles is day,
								 * next 2 nibbles is month, last 2 nibbles is year	*/
	uint8_t		rtcTime[3];		/* 3 bytes, 6 nibbles, first 2 nibbles is hours,
								 * next 2 nibbles is minutes, last 2 nibbles is seconds */
	uint8_t		gsmSig;
	uint16_t	gsmVolt;

	uint8_t		gpsLonLat[9];	/* 9 bytes, 18 nibbles, first 9 nibbles is
								 * longitude, last 9 nibbles is latitude */

#if HAVE_WEIGHT_SENSOR == 1
	uint8_t		bhWeight[5];
#endif
} datablock_t;
	


#endif	/* __DATA_H__ */
