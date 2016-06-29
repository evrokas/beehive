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

void getTime(datetime_t *dt)
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
    Serial.print("0");
    
  Serial.print(hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (minute<10)
  {
    Serial.print("0");
  }
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second<10)
  {
    Serial.print("0");
  }
  Serial.print(second, DEC);
  Serial.print(" ");
  if(dayOfMonth < 10)
    Serial.print("0");
    
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  if(month<10)
    Serial.print("0");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(year, DEC);
  Serial.print(" Day of week: ");
  switch(dayOfWeek){
  case 1:
    Serial.println("Sunday");
    break;
  case 2:
    Serial.println("Monday");
    break;
  case 3:
    Serial.println("Tuesday");
    break;
  case 4:
    Serial.println("Wednesday");
    break;
  case 5:
    Serial.println("Thursday");
    break;
  case 6:
    Serial.println("Friday");
    break;
  case 7:
    Serial.println("Saturday");
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
