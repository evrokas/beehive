
#include "bms.h"
#include "../utils.h"
#include "../rtc.h"

void setup()
{
#if 1

	delay( 2000 );
	Serial.begin( 9600 );
	setupPeripheralsControl();

	/* run this code only to setup date and time */
	powerPeripherals( 1, 1 );
	powerRTC(1, 10);
	setTime(0,42, 01, 4, 20, 9, 17 );
	powerPeripherals( 0, 0 );

	/* stop program execution */
	Serial.println("Now you must comment out RTC writing code, and reload program to board.\n");
	while(true);
#endif


}


void loop()
{
	while( true );
}
