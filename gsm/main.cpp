/*
 * GSM/GPRS module testing
 */

#include <stdlib.h>

#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>

//#include <Sim800l.h>

#include "bms.h"
#include "utils.h"

SoftwareSerial	gsm(9,10);

//TinyGsm gsmi( gsm );

unsigned long mil1, mil2;

void write_gsm(char *str)
{
	while( *str != '\0') {
		gsm.write( *str );
		str++;
		delay(10);
	}
}


void sapbr_init()
{
	write_gsm("AT+CIPSHUT\n");
	write_gsm("AT+CIPMUX=0\n");
	write_gsm("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\n");
	write_gsm("AT+SAPBR=3,1,\"APN\",\"internet.cyta.gr\"\n");
	write_gsm("AT+SAPBR=3,1,\"USER\",\"\"\n");
	write_gsm("AT+SAPBR=3,1,\"PWD\",\"\"\n");
	write_gsm("AT+SAPBR=1,1\n");
	write_gsm("AT+SAPBR=2,1\n");
}


/*
http://46.176.29.74:8088/data.php?action=add&apikey=abcdefgh&nodeId=1088&mcuTemp=60&batVolt=3.998&bhvTemp=31.345&bhvHumid=45.432&rtcDateTime=13-03-17_16:53&gsmSig=45&gsmVolt=4.023&gpsLon=12.345232&gpsLat=45.123433&bhvWeight=45.567

http://46.176.29.74:8088/data.php?action=add&1=abcdefgh&2=1088&3=3.998&4=31.345&5=45.432&6=13-03-17_16:53&7=45&8=4.023&9=12.345232&10=45.123433&11=45.567

if we change &[values] to &[number 1-11]

mcuTemp is deprecate therefore it is removed.

*/

void http_send()
{
	write_gsm("AT+HTTPINIT\n");
	write_gsm("AT+HTTPPARA=\"CID\",1\n");
	write_gsm("AT+HTTPPARA=\"URL\",\"http://46.176.29.74:8088/data.php?action=add&apikey=abcdefgh&nodeId=1088&mcuTemp=60&batVolt=3.998&bhvTemp=31.345&bhvHumid=45.432&rtcDateTime=13-03-17_16:53&gsmSig=45&gsmVolt=4.023&gpsLon=12.345232&gpsLat=45.123433&bhvWeight=45.567\"\n");
	write_gsm("AT+HTTPACTION=0\n");
	write_gsm("AT+HTTPREAD\n");
}





void setup()
{
  setupPeripheralsControl();

  // reset peripherals
  powerPeripherals( 0,0 );
  delay( 1000 );

  // and leave them on
  powerPeripherals( 1,1 );

//  powerGPRSGPS(1);

  Serial.begin( 9600 );

  gsm.begin( 9600 );
  delay( 1000 );
  
//  gps.listen();

  Serial.println("\n\nTinyGSM example.");
  mil1 = millis();
  
  
  powerPeripherals( 0,0 );
  powerGPRSGPS( 0 );
}


unsigned char gsmon=0;
unsigned char peron=0;

void loop()
{
  char c;
  uint8_t i;
  unsigned long volt;
    
  if(Serial.available()) {
    c=Serial.read();
    switch( c ) {
    	case '!': 
    		if(gsmon == 1)gsmon = 0;
    		else gsmon = 1;
		
    		powerGPRSGPS( gsmon );
    		Serial.print("GSM/GPRS ... ");
			Serial.println( gsmon?"on":"off" );
    		break;
		case '@':
			if(peron == 1)peron = 0;
			else peron = 1;
		
			powerPeripherals( peron, peron );
			Serial.print("Peripherals ... ");
			Serial.println( peron?"on":"off" );
			break;
		case '#':
			sapbr_init();
			break;
		case '$':
			http_send();
			break;
		case '%':
			Serial.print("Vcc: ");
			
			for(i=0;i<10;i++) {
				volt += readVcc();
				delay(5);
			}
			Serial.println( volt / 10 );
			break;

		default:
			gsm.write( c );
	}
  }
  
  if(gsm.available() ) {
    Serial.write( gsm.read() );
  }

}

/*
  

#if 0
  if(Serial.available()) {
    char g;
      g = Serial.read();

      switch(g) {
        case '1': dump1=not dump1; break;
        case '2': dump2=not dump2; break;
      default:
        gps.write( g );
      }
  }
#endif
    
  if(gps.available()) {
    char c;
    
      c = gps.read();
      if(dump1)
        Serial.write( c );

#if 1
      if( gpsi.encode( c ) ) {
        newData = true;
      }
#endif

  }
  
  if( newData ) {

    float flat, flon;
    unsigned long age;


      mil2 = millis();
      Serial.print("millis diff: ");  Serial.println( mil2 - mil1);
      newData = false;
      gpsi.f_get_position(&flat, &flon, &age);
      Serial.print("LAT=");
      Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
      Serial.print(" LON=");
      Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
      Serial.print(" SAT=");
      Serial.print(gpsi.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gpsi.satellites());
      Serial.print(" PREC=");
      Serial.println(gpsi.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gpsi.hdop());
  }
  
  
  if( dump2 ) {
    gpsi.stats(&chars, &sentences, &failed);
    Serial.print(" CHARS=");
    Serial.print(chars);
    Serial.print(" SENTENCES=");
    Serial.print(sentences);
    Serial.print(" CSUM ERR=");
    Serial.println(failed);
    if (chars == 0)
      Serial.println("** No characters received from GPS: check wiring **");
  }

}
*/
