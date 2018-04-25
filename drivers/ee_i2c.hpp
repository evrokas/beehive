
/*
 * I2C EEPROM read/write routines
 *
 * author Evangelos Rokas <evrokas@gmail.com>
 *
 */

#ifndef __EE_I2C_HPP__
#define __EE_I2C_HPP__


extern uint8_t	__ee_dev_addr;
extern uint32_t	__ee_dev_size;

void __ee_init(uint8_t addr, uint16_t dsize)
{
	__ee_dev_addr = addr;
	__ee_dev_size = dsize * 128UL;	// 128 = 1024UL / 8;
}


void __ee_end()
{
	__ee_dev_addr = 0;
	__ee_dev_size = 0;
}


void __ee_write(uint16_t addr, uint8_t data ) 
{
  Wire.beginTransmission(__ee_dev_addr);
  Wire.write((uint16_t)(addr >> 8));   // MSB
  Wire.write((uint16_t)(addr & 0xFF)); // LSB
  Wire.write(data);
  Wire.endTransmission();
 
  delay(5);
}


uint8_t __ee_read(uint16_t addr ) 
{
  uint8_t rdata = 0xFF;
 
  Wire.beginTransmission(__ee_dev_addr);
  Wire.write((uint16_t)(addr >> 8));   // MSB
  Wire.write((uint16_t)(addr & 0xFF)); // LSB
  Wire.endTransmission();
 
  Wire.requestFrom((int)__ee_dev_addr,1);
 
  if (Wire.available()) rdata = Wire.read();
 
  return rdata;
}

uint8_t __ee_readblock(uint16_t addr, char *data, uint8_t datalen)
{
  uint8_t i=0;
  
  Wire.beginTransmission(__ee_dev_addr);
  Wire.write((uint16_t)(addr >> 8));   // MSB
  Wire.write((uint16_t)(addr & 0xFF)); // LSB
  Wire.endTransmission();
 
  Wire.requestFrom(__ee_dev_addr, datalen);
 

//  Serial.print(F("__ee_readblock len:"));Serial.println(datalen);
  
  for(i=0;i<datalen;i++)
		if (Wire.available()) {
  		data[i] = Wire.read();
//  		Serial.print(data[i]);
		}
 
  return i;
}


void __ee_writeblock(uint16_t addr, char *data, uint8_t datalen)
{
	uint8_t i;

  Wire.beginTransmission(__ee_dev_addr);
  Wire.write((uint16_t)(addr >> 8));   // MSB
  Wire.write((uint16_t)(addr & 0xFF)); // LSB

//  Serial.print(F("__ee_writeblock len:"));Serial.println(datalen);
  for(i=0;i<datalen;i++) {
  	Wire.write(data[i]);
//  	Serial.print(data[i]);
	}
  Wire.endTransmission();
 
  delay(5);
}

#endif	/* __EE_I2C_HPP__ */
