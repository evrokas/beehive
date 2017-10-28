
/*
 * dummy read/write routines for testing
 *
 * author Evangelos Rokas <evrokas@gmail.com>
 *
 */

#ifndef __EE_DUMMY_HPP__
#define __EE_DUMMY_HPP__


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
  delay(5);
}


uint8_t __ee_read(uint16_t addr ) 
{
  return 0;
}

uint8_t __ee_readblock(uint16_t addr, char *data, uint8_t datalen)
{
  uint8_t i=0;
  
  for(i=0;i<datalen;i++)
  	data[i] = 0;
 
  return i;
}


void __ee_writeblock(uint16_t addr, char *data, uint8_t datalen)
{
  delay(5);
}

#endif	/* __EE_DUMMY_HPP__ */
