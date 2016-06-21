/*
 * rtc.h -Real-Time Clock header
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-16
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __RTC_H__
#define __RTC_H__

#define RTC_I2C_ADDRESS 0x68

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  byte second;
  byte minute;
  byte hour;
  byte dayOfWeek;
  byte dayOfMonth;
  byte month;
  byte year;
} datetime_t;

/* set the time to the RTC */
void setTime(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte yr);

/* read time from the RTC */
void readTime(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *yr);

/* get seconds of the day from the start of the month */
unsigned long getSeconds();

/* print current date and time to serial output */
void displayTime();	

void getTime(datetime_t *dt);

void convertTime2Str(char *str, datetime_t *dt);

#ifdef __cplusplus
}
#endif


#endif /* __RTC_H__ */
