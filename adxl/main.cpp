
#include <Arduino.h>
#include <Wire.h>
#include "../utils.h"
#include "../accel.h"


void setup()
{
    setupPeripheralsControl();
    powerPeripherals(1,1);

    accel_init();
    Wire.begin();
    Serial.begin( 9600 );
    

    Serial.println("Accelerator reading values...\n");
    Serial.flush();
    
}


#if 1
void loop()
{
  int16_t ax, ay, az;

    while(1) {
        delay(100);
        
        Serial.print("> ");
        accel_getxyz(&ax, &ay, &az);
        
        Serial.print(ax); Serial.print("\t"); Serial.print(ay); Serial.print("\t"); Serial.println(az);
    }
}
#else
void loop()
{
}
#endif
