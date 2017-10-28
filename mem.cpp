/*
 * mem.cpp - memory module functions file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-17
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

//#include <extEEPROM.h>

#if LINUX_NATIVE_APP == 1

#include <stdio.h>

#else

#include <Wire.h>

#endif

 
#include "bms.h"
#include "mem.h"


int32_t	__head_db;		/* head pointer of datablocks */
int32_t	__tail_db;		/* tail pointer of datablocks */
int32_t	__max_db;			/* maximum data block available */
int16_t __cnt_db;			/* number of data blocks in storage */


int32_t	__begin_addr;	/* first EEPROM address to access */
int32_t	__end_addr;		/* last EEPROM address to access */


uint8_t		__ee_dev_addr;
uint32_t	__ee_dev_size;



void mem_stats()
{
#if LINUX_NATIVE_APP == 1
	fprintf(stderr, "<mem stats: cnt=%i\thead=%i\ttail=%i>\n",
		__cnt_db, __head_db, __tail_db );
#else
	Serial.print( "<mem stats: cnt="); Serial.print( __cnt_db );
	Serial.print( " head=" ); Serial.print(__head_db);
	Serial.print( " tail=" ); Serial.print(__tail_db);
	Serial.println(">");
#endif
}


/* dev_size: eeprom size in kbits, dev_addr, device address (default: 0x50)
 */
 
void mem_init(uint32_t dev_size, uint8_t dev_addr = 0x50)
{
	__ee_init(dev_addr, dev_size);
	
//	__ee_dev_addr = dev_addr;
//	__ee_dev_size = dev_size * 1024UL / 8;

	__head_db = -1;
	__tail_db = 0;
	__max_db = __ee_dev_size / BLOCK_SIZE;

	__cnt_db = 0;
	
	__begin_addr = 0;
	__end_addr = (uint32_t)dev_size * 1024UL / 8;			/* this is for 24C32 memory chip, found in DS3231 breakout board */
	
	
#if LINUX_NATIVE_APP == 1
	fprintf(stderr, "ee_dev_size: %i\t ee_dev_addr=0x%02x\n", __ee_dev_size, __ee_dev_addr);
#else
	Serial.print("ee_dev_size: ");
	Serial.print(__ee_dev_size, DEC);
	Serial.print("\tee_dev_addr: 0x");
	Serial.println(__ee_dev_addr, HEX);
#endif

}

void mem_end()
{
	__ee_end();

	__cnt_db = 0;
}


uint32_t	__block2linearaddress(uint16_t blockno)
{
	return ( blockno * BLOCK_SIZE );
}

uint16_t __linearaddress2block(uint32_t linaddress)
{
	return (linaddress / BLOCK_SIZE );
}


bool mem_write(const void *p, uint8_t psize, uint16_t blockno)
{
#if LINUX_NATIVE_APP == 1
	fprintf(stderr, "mem_write (blockno: %i, linaddr=%i)\n", blockno, __block2linearaddress(blockno));
#else
	Serial.print("mem_write (blockno: ");
	Serial.print( blockno, DEC );
	Serial.print("\tlinadd= ");
	Serial.println( __block2linearaddress(blockno) );
#endif

	__ee_writeblock( __block2linearaddress( blockno ), (char *)p, psize );
	return true;
}

bool mem_read(const void *p, uint8_t psize, uint16_t blockno)
{
#if LINUX_NATIVE_APP == 1
	fprintf(stderr, "mem_read (blockno: %i, linaddr=%i)\n", blockno, __block2linearaddress( blockno ));
#else
	Serial.print("mem_read (blockno: ");
	Serial.print( blockno, DEC );
	Serial.print("\tlinadd= ");
	Serial.println( __block2linearaddress(blockno) );
#endif

	if( __ee_readblock( __block2linearaddress( blockno ), (char *)p, psize ) == psize)
		return true;
	else
		return false;
}

bool mem_pushDatablock(datablock_t *db)
{
#if defined(LINUX_NATIVE_APP)
	fprintf(stderr, "mem_pushDatablock\n");
#else	
	Serial.println("mem_pushDatablock");
#endif
	
	if(__cnt_db >= __max_db)return false;
	if((__head_db == -1) && (__cnt_db == 0)) {
		__head_db = 0;
		__tail_db = 0;
	}

	mem_write(db, sizeof( datablock_t ), __head_db );

//	if(__cnt_db == 0)__tail_db = __head_db;
	__cnt_db++;
	
	/* increase __head_db taking care of overflow */
	__head_db = (__head_db + 1) % __max_db;
	

#if LINUX_NATIVE_APP == 1
#else
	Serial.print("push: new __head_db: ");
	Serial.println( __head_db, DEC );
#endif
	
	return true;
}

bool mem_popDatablock(datablock_t *db)
{
#if defined(LINUX_NATIVE_APP)
	fprintf(stderr, "mem_popDatablock\n");
#else
	Serial.println("mem_popDatablock");
#endif

	if(!__cnt_db)return false;
	
	mem_read(db, sizeof( datablock_t ), __tail_db);

	__tail_db = (__tail_db + 1) % __max_db;
	__cnt_db--;

#if LINUX_NATIVE_APP == 1
#else
	Serial.print("pop: new __tail_db: ");
	Serial.println( __tail_db, DEC );
#endif

		
	return true;
}
