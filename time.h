/*
 * time.h - time keeping header
 *
 * BEEWatch - Beehive Monitoring System
 * (c) Evangelos Rokas, 2015-18
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __TIME_H__
#define __TIME_H__


#include <stdint.h>


//#define DEBUG



/* bit manipulation macros
 * source: http://www.mathcs.emory.edu/~cheung/Courses/255/Syllabus/1-C-intro/bit-array.html
 */

#define SetBit(A,k)     ( A[(k/8)] |= (1 << (k%8)) )
#define ClearBit(A,k)   ( A[(k/8)] &= ~(1 << (k%8)) )
#define TestBit(A,k)    ( A[(k/8)] & (1 << (k%8)) )

/* define DAYS_IN_TIME if you need to add day time keeping
 * functionality */
#define DAYS_IN_TIME	1


typedef struct {
	uint8_t min[8];
	uint8_t hr[3];

#ifdef DAYS_IN_TIME
	uint8_t day[1];
#endif

} timekeeping_t;


#ifdef __cplusplus
extern "C" {
#endif

void clearTime(timekeeping_t &tk);
bool initTime(timekeeping_t &tk, uint8_t hr, uint8_t min, uint8_t dy = 0);
bool checkTime(timekeeping_t &tk, uint8_t hr, uint8_t min, uint8_t dy = 0);
bool findNextTime(timekeeping_t &tk, uint8_t hr, uint8_t min, uint8_t dy, uint8_t &hh, uint8_t &mm, uint8_t &dd);

#ifdef LINUX_NATIVE_APP
void printTime(timekeeping_t &tk, uint8_t flags = 0);
void printTimeJSON(timekeeping_t &tk);
void printTimeBinary(timekeeping_t &tk);
#endif

#ifdef __cplusplus
};	/* extern "C" */
#endif

#ifdef LINUX_NATIVE_APP

#	define	PUTCHAR(c)	printf("%s", c);
# define	PUTCHARX(c)	printf("%02x", c)
# define	PUTCHAR2(c)	printf("0x%02x", c)

#else

#if 0
#	define	PUTCHAR(c)	Serial.write(c)
# define	PUTCHARX(c) Serial.write((unsigned char)c, HEX)
# define  PUTCHAR2(c)	Serial.write("0x"); Serial.write((unsigned char)c, HEX)
#endif

#endif	/* LINUX_NATIVE_APP */


#endif	/* __TIME_H__ */
