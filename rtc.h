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

#if 0
#ifndef LINUX_NATIVE_APP
#include <WString.h>
#endif
#endif

#if 0

#ifndef byte
#define byte	char
#endif

#endif

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

void rtc_init();
void rtc_done();

void rtc_getTime(datetime_t *dt);

/* get minutes of the day from the start of the month */
unsigned long rtc_getMinutes(datetime_t *dt);

/* print current date and time to serial output */
void displayTime();	

void setTime(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte yr);

#if 0
void convertTime2Str(char *str, datetime_t *dt);
void convertDate2Str(char *str, datetime_t *dt);
#endif

static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d);
static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s);
uint32_t unixtime(datetime_t *dt);
long secondstime(datetime_t *dt);


#ifdef __cplusplus
}
#endif


#endif /* __RTC_H__ */
