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
void printTime(timekeeping_t &tk);

#ifdef __cplusplus
};	/* extern "C" */
#endif

#ifdef LINUX_NATIVE_APP

#	define	PUTCHAR(c)	putchar(c)

#else

#	define	PUTCHAR(c)	Serial.write(c)

#endif	/* LINUX_NATIVE_APP */


#endif	/* __TIME_H__ */
