
#include <Arduino.h>
#include <Wire.h>
#include "../utils.h"
#include "../accel.h"


volatile char adxlActivity = 0;

void adxl_activity()
{
  if(accel_getactivitysource())
    adxlActivity = 1;
}


void setup()
{
    setupPeripheralsControl();
    powerPeripherals(0,0);
    delay( 1000 );
    powerPeripherals(1,1);

    accel_init();
    Wire.begin();
    Serial.begin( 9600 );
    

    Serial.println("Accelerator reading values...\n");
//    Serial.flush();

    attachInterrupt(0, adxl_activity, RISING);
    
    accel_getactivitysource();
}




void loop()
{
  int16_t ax, ay, az;
  float roll, pitch;

    while(1) {
        delay(100);
        
        if(adxlActivity) {
          Serial.print("A ");
          adxlActivity=0;
        } else {
          Serial.print("> ");
        }
        accel_getxyz(&ax, &ay, &az);
        accel_calculaterollpitch(ax, ay, az, &roll, &pitch);
        
        Serial.print(ax); Serial.print("\t"); Serial.print(ay); Serial.print("\t"); Serial.print(az); Serial.print("\t");
        Serial.print(roll); Serial.print("\t"); Serial.println(pitch);
    }
}
