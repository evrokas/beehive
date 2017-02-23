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

#include <WString.h>

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

void convertTime2Str(char *str, datetime_t *dt);
void convertDate2Str(char *str, datetime_t *dt);

#ifdef __cplusplus
}
#endif


#endif /* __RTC_H__ */
