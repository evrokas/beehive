/*
 * eeprom.ino - eeprom data file
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
#include <EEPROM.h>

EEMEM	unsigned char data = 0xab;
EEMEM	unsigned int data2 = 0xfedc;

EEMEM	unsigned char tst[] = {1, 2, 3, 4, 5};
EEMEM	unsigned char tst2[] = {101, 102, 103, 104, 105,
								106,107,108,109,110,111,112};


void setup()
{
	Serial.begin(9600);
}

void loop()
{
	Serial.write( tst[0] );
	Serial.write( tst2[3] );
}
