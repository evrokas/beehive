
#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#include "bms.h"
#include "utils.h"

SoftwareSerial	gps(6,7);
TinyGPS gpsi;

unsigned long mil1, mil2;

void setup()
{
  setupPeripheralsControl();

  // reset peripherals
  powerPeripherals( 0,0 );
  delay( 1000 );

  // and leave them on
  powerPeripherals( 1,1 );

  Serial.begin( 9600 );

  gps.begin( 9600 );
//  gps.listen();

  Serial.println("\n\nTinyGPS example.");
  mil1 = millis();
}


static bool dump1 = false;
static bool dump2 = false;

void loop()
{
  bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

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
