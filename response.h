/*
 * response.h - parse server response header
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-18
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __RESPONSE_H__
#define __RESPONSE_H__


#define RESPONSE_STR_SIZE	32

#define raNONE				0x0
#define raSETNODE			0x1
#define raSETDATASERVER		0x2
#define raSETSERVERPORT		0x3
#define raSETAPN			0x4
#define raSETUSER			0x5
#define raSETPASS			0x6
#define raSETLOGPROG		0x7
#define raSETNETPROG		0x8
#define raSETAPIKEY			0x9
#define raSETACTIVE			0xa
#define raSETDST 			0xb
#define raSETDNS			0xc
#define raSETRTC			0xd
#define raNONE1				0xe
#define raSTOP 				0xf		/* this marks the end of command stream */

#define rsINIT	0x0
#define rsCMD	0x1
#define rsPAY 	0x2



void initResponseParser();
bool parseResponse(char ch);


#endif	/* __RESPONSE_H__ */