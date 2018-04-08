/*
 * rtc.cpp - Real-Time Clock functions file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-16
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */


#include <Arduino.h>
#include <Wire.h>
#include <avr/pgmspace.h>

#include "bms.h"
#include "rtc.h"

/* helper functions
 * Convert normal decimal numbers to binary coded decimal */
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}

/* Convert binary coded decimal to normal decimal numbers */
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

void rtc_init()
{
  Wire.begin();
}

void rtc_done()
{
}


/*
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  // set the initial time here:
  // DS3231 seconds, minutes, hours, day, date, month, year
  // setTime(30,42,21,4,26,11,14);
}
*/


void setTime(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte yr)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(yr)); // set year (0 to 99)
  Wire.endTransmission();
}

void readTime(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *yr)
{
  Wire.beginTransmission(RTC_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(RTC_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec( Wire.read() & 0x7f );
  *minute = bcdToDec( Wire.read() );
  *hour = bcdToDec( Wire.read() & 0x3f );
  *dayOfWeek = bcdToDec( Wire.read() );
  *dayOfMonth = bcdToDec( Wire.read() );
  *month = bcdToDec( Wire.read() );
  *yr = bcdToDec( Wire.read() );
}

unsigned long getSeconds()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  unsigned long secs;

  readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,&year);
  
  secs = second + minute * 60 + hour * 3600 + dayOfMonth * 86400;
  return (secs);
}

unsigned long rtc_getMinutes(datetime_t *dt)
{
  unsigned long mins;

  mins = dt->minute + dt->hour * 60 + dt->dayOfMonth * 1440;
  return (mins);
}



void rtc_getTime(datetime_t *dt)
{
  readTime(&dt->second, &dt->minute, &dt->hour, &dt->dayOfWeek, &dt->dayOfMonth, &dt->month, &dt->year);
}


void displayTime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readTime(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,&year);
  // send it to the serial monitor
  if(hour<10)
    Serial.print( F("0") );
    
  Serial.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print( F(":") );
  if (minute<10)
  {
    Serial.print( F("0") );
  }
  Serial.print(minute, DEC);
  Serial.print( F(":") );
  if (second<10)
  {
    Serial.print( F("0") );
  }
  Serial.print(second, DEC);
  Serial.print( F(" ") );
  if(dayOfMonth < 10)
    Serial.print( F("0") );
    
  Serial.print(dayOfMonth, DEC);
  Serial.print( F("/") );
  if(month<10)
    Serial.print( F("0") );
  Serial.print(month, DEC);
  Serial.print( F("/") );
  Serial.print(year, DEC);
  Serial.print(F(" Day of week: "));
  switch(dayOfWeek){
  case 1:
    Serial.println(F("Sunday"));
    break;
  case 2:
    Serial.println(F("Monday"));
    break;
  case 3:
    Serial.println(F("Tuesday"));
    break;
  case 4:
    Serial.println(F("Wednesday"));
    break;
  case 5:
    Serial.println(F("Thursday"));
    break;
  case 6:
    Serial.println(F("Friday"));
    break;
  case 7:
    Serial.println(F("Saturday"));
    break;
  }
}


void convertTime2Str(char *str, datetime_t *dt)
{
/*
  sprintf(str, "%02d-%02d-%02d %02d:%02d:%02d", 
    dt->dayOfMonth, dt->month, dt->year,
    dt->hour, dt->minute, dt->second
  );
*/
  sprintf(str, "%02d:%02d:%02d", dt->hour, dt->minute, dt->second);
}

void convertDate2Str(char *str, datetime_t *dt)
{
  sprintf(str, "%02d-%02d-%02d", dt->dayOfMonth, dt->month, dt->year);
}



////////////////////////////////////////////////////////////////////////////////
// utility code, some of this could be exposed in the DateTime API if needed

#define SECONDS_FROM_1970_TO_2000 946684800


uint32_t unixtime(datetime_t *dt)
{
	return secondstime( dt ) + SECONDS_FROM_1970_TO_2000;
}
    
long secondstime(datetime_t *dt)
{
	uint32_t t;
	uint16_t days = date2days(dt->year,dt->month, dt->dayOfMonth);

		t = time2long(days, dt->hour, dt->minute, dt->second);    
	
	return t;
}

const static uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    
// number of days since 2000/01/01, valid for 2001..2099
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d)
{
	if (y >= 2000)
  	y -= 2000;
	
	uint16_t days = d;
	for (uint8_t i = 1; i < m; ++i)
  	days += pgm_read_byte(daysInMonth + i - 1);
	if (m > 2 && y % 4 == 0)
  	++days;
	
	return days + 365 * y + (y + 3) / 4 - 1;
}
    
static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s)
{
	return ((days * 24L + h) * 60 + m) * 60 + s;
}

