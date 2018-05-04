
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
	Fastwire::beginTransmission(__ee_dev_addr);
  Fastwire::write((uint16_t)(addr >> 8));   // MSB
  Fastwire::write((uint16_t)(addr & 0xFF)); // LSB
  Fastwire::write(data);
  Fastwire::endTransmission();
 
  delay(5);
}


uint8_t __ee_read(uint16_t addr ) 
{
  uint8_t rdata = 0xFF;
 
  Fastwire::beginTransmission(__ee_dev_addr);
  Fastwire::write((uint16_t)(addr >> 8));   // MSB
  Fastwire::write((uint16_t)(addr & 0xFF)); // LSB
  Fastwire::endTransmission();
 
  Fastwire::requestFrom((int)__ee_dev_addr,1);
 
  if (Fastwire::available()) rdata = Fastwire::read();
 
  return rdata;
}

uint8_t __ee_readblock(uint16_t addr, char *data, uint8_t datalen)
{
  uint8_t i=0,l;
  
  Fastwire::beginTransmission(__ee_dev_addr);
  Fastwire::write((uint16_t)(addr >> 8));   // MSB
  Fastwire::write((uint16_t)(addr & 0xFF)); // LSB
  Fastwire::endTransmission();
 
  l = Fastwire::requestFrom(__ee_dev_addr, datalen);
  
 	if(l != datalen) {
 		Serial.println(F("could not read requested number of bytes from I2C"));
 		return (l);
	} 

//  Serial.print(F("__ee_readblock len:"));Serial.println(datalen);
  
  for(i=0;i<datalen;i++)
		if (Fastwire::available()) {
  		data[i] = Fastwire::read();
//  		Serial.print(data[i]);
		}
 
  return i;
}


void __ee_writeblock(uint16_t addr, char *data, uint8_t datalen)
{
	uint8_t i;

  Fastwire::beginTransmission(__ee_dev_addr);
  Fastwire::write((uint16_t)(addr >> 8));   // MSB
  Fastwire::write((uint16_t)(addr & 0xFF)); // LSB

//  Serial.print(F("__ee_writeblock len:"));Serial.println(datalen);
  for(i=0;i<datalen;i++) {
  	Fastwire::write(data[i]);
//  	Serial.print(data[i]);
	}
  Fastwire::endTransmission();
 
  delay(5);
}

#endif	/* __EE_I2C_HPP__ */
