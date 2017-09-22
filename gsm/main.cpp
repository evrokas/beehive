/*
 * GSM/GPRS module testing
 */

#include <stdlib.h>

#include <Arduino.h>
#include <Wire.h>
#include "LowPower.h"
//#include <Sim800l.h>
#include <extEEPROM.h>

#include "bms.h"
#include "utils.h"
#include "../mem.h"
#include "../rtc.h"
#include "../gsm.h"

//#include "../drivers/ee_i2c.hpp"

//#define USE_NEWSWSERIAL

#ifndef USE_NEOSWSERIAL

#include <SoftwareSerial.h>
//SoftwareSerial	gsm(9,10);

#else

#include "NeoSWSerial.h"
//NeoSWSerial			gsm(9, 10);

#endif


//TinyGsm gsmi( gsm );

unsigned long mil1, mil2;

void write_gsm(char *str)
{
	while( *str != '\0') {
		gsm_write( *str );
		str++;
		//delay(10);
	}
}

#define CF(str)	(char *)( str )


void sapbr_init()
{
#if 0

	write_gsm("AT+CIPSHUT\n");
	write_gsm("AT+CIPMUX=0\n");
#endif


#if 1
	if( gsm_activateBearerProfile(CF("internet.cyta.gr"), CF(""), CF("") ) ) {
		Serial.println("GSM: bearer profile is activated!\n");
	} else {
		Serial.println("GSM: bearer profile is *NOT* activated!\n");
	}
#endif

#if 0
	write_gsm("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\n");
	write_gsm("AT+SAPBR=3,1,\"APN\",\"internet.cyta.gr\"\n");
	write_gsm("AT+SAPBR=3,1,\"USER\",\"\"\n");
	write_gsm("AT+SAPBR=3,1,\"PWD\",\"\"\n");
	write_gsm("AT+SAPBR=1,1\n");
	write_gsm("AT+SAPBR=2,1\n");
#endif

}


/*
http://46.176.29.74:8088/data.php?action=add&apikey=abcdefgh&nodeId=1088&mcuTemp=60&batVolt=3.998&bhvTemp=31.345&bhvHumid=45.432&rtcDateTime=13-03-17_16:53&gsmSig=45&gsmVolt=4.023&gpsLon=12.345232&gpsLat=45.123433&bhvWeight=45.567

http://46.176.29.74:8088/data.php?action=add&1=abcdefgh&2=1088&3=3.998&4=31.345&5=45.432&6=13-03-17_16:53&7=45&8=4.023&9=12.345232&10=45.123433&11=45.567

if we change &[values] to &[number 1-11]

mcuTemp is deprecate therefore it is removed.

*/

void http_send()
{
	write_gsm("AT+HTTPINIT\r\n");
	write_gsm("AT+HTTPPARA=\"CID\",1\r\n");
	write_gsm("AT+HTTPPARA=\"URL\",\"http://5.55.150.188:8088/data.php?action=add&apikey=abcdefgh&nodeId=1088&mcuTemp=60&batVolt=3.998&bhvTemp=31.345&bhvHumid=45.432&rtcDateTime=13-03-17_16:53&gsmSig=45&gsmVolt=4.023&gpsLon=12.345232&gpsLat=45.123433&bhvWeight=45.567\"\n");
	write_gsm("AT+HTTPACTION=0\r\n");
	write_gsm("AT+HTTPREAD\r\n");
}

extEEPROM	ee(kbits_256, 1, 64, 0x57);




void setup()
{
	uint8_t eepst;
	
  setupPeripheralsControl();
  // reset peripherals
  powerPeripherals( 1,1 );

  Serial.begin( 9600 );
  gsm_init( );

  powerRTC( 1, 10 );

  Serial.println("ee.begin()\n"); Serial.flush();

	if( (eepst=ee.begin( extEEPROM::twiClock100kHz )) ) {
		Serial.print( "Could not initialize external EEPROM. Status = 0x" );
		Serial.println( eepst, HEX );
	}
	
	
//  gps.listen();

  Serial.println("\n\nTinyGSM example.");
  mil1 = millis();
  
  
  powerPeripherals( 0,0 );
  powerGPRSGPS( 0 );
  powerRTC( 0, 1 );

  mem_init( 32, /*256,*/ (0x57) );
}


unsigned char gsmon=0;
unsigned char peron=0;
unsigned char rtcon=0;
char tempbuf[32];

void loop()
{
  char c;
  uint8_t i;
  uint16_t i16;
  unsigned long volt;
  datetime_t dt;
    
  if(gsm_available() ) {
    Serial.write( gsm_read() );
  }



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
		case '^':
			Serial.print("Entering power down mode ... "); Serial.flush();
			LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
			Serial.println("done!");
			break;
		case '&':
			Serial.print("Reading time ... ");
			displayTime();
//			rtc_getTime( &dt );
			//Serial.println( dt );
			break;
		case '*':
			if(rtcon == 1)rtcon = 0;
			else rtcon = 1;
			
			Serial.print("RTC ... ");
			powerRTC(rtcon, 1);
			Serial.println( rtcon ? "on":"off" );
			break;
		case '~':
			memset(tempbuf, 0, sizeof( tempbuf ));
			strcpy(tempbuf, "");
			
			Serial.print("enter cmd: ");
			
			do {
				while( !Serial.available() ) ;
				c = Serial.read();
				Serial.print(c);
				if(c == '\r') {
				  uint8_t bln;
					char *cc;
					  
					Serial.print("\r\n>>> ");
					Serial.println( tempbuf+1 );
					
					switch( tempbuf[0] ) {
						case 'r':
							bln = atoi( tempbuf+1 );
							Serial.print( "EEPROM: read offset: 0x" ); Serial.print( bln * 32, HEX ); Serial.print( ": " );
//							c = ee.read( bln * 32 );
							c = __ee_read( bln * 32UL );
							Serial.println( (char)c, HEX );
							break;
						case 'w':
							bln = atoi( tempbuf+1 );
							cc = strchr(tempbuf, ',') + 1;
							c = atoi( cc );
							Serial.print( "EEPROM: write 0x");
							Serial.print( c, HEX ); 
							Serial.print(" at offset: 0x" ); Serial.println( bln * 32, HEX );
//							ee.write(bln * 32, c );
							__ee_write( bln * 32UL, c );

							c = 0;
							Serial.print( "EEPROM: read offset: 0x" ); Serial.print( bln * 32, HEX ); Serial.print( ": " );
//							c = ee.read( bln * 32 );
							c = __ee_read( bln * 32UL );
							Serial.println( (char)c, HEX );
							break;
						case 's':
							c = (tempbuf[1] - '0') * 16 + (tempbuf[2] - '0');
							Serial.print("Selecting EEPROM device: 0x"); Serial.println( c, HEX);
//							__ee_setaddr( c );
							break;

						case 'z':
							Serial.print("Bearer status: ");
							i = gsm_getBearerStatus();
							Serial.println( (char)('0' + i) );
							break;
						
						case 'x':
							Serial.print("Battery: ");
							if( gsm_getBattery( i16 ) )
								Serial.println( i16 );
							else
								Serial.println("could not read battery levels!");
							break;

						default:;
					}
					
						
						

					break;
				}
				strncat(tempbuf, &c, 1);
			} while(1);
			break;			
		default:
			gsm_write( c );
	}
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
