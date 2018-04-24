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
#include "utils.h"


counter_type	__head_db;		/* head pointer of datablocks */
counter_type	__tail_db;		/* tail pointer of datablocks */
counter_type	__max_db;			/* maximum data block available */
counter_type	__cnt_db;			/* number of data blocks in storage */


counter_type	__begin_addr;	/* first EEPROM address to access */
counter_type	__end_addr;		/* last EEPROM address to access */


uint8_t		__ee_dev_addr;
uint32_t	__ee_dev_size;


// set DEBUG_MEM to 1 in order to emit the debugging messages
#define DEBUG_MEM	1


void mem_stats()
{
#if LINUX_NATIVE_APP == 1
	fprintf(stderr, "<mem stats: cnt=%i\thead=%i\ttail=%i>\n",
		__cnt_db, __head_db, __tail_db );
#else
	Serial.print( F("<mem stats: cnt=") ); Serial.print( __cnt_db );
	Serial.print( F(" head=") ); Serial.print(__head_db);
	Serial.print( F(" tail=") ); Serial.print(__tail_db);
	Serial.println( F(">") );
#endif
}


/* dev_size: eeprom size in kbits, dev_addr: device address (default: 0x50)
 */
 
void mem_init(uint32_t dev_size, uint8_t dev_addr = 0x50)
{
	if(sizeof( datablock_t ) != BLOCK_SIZE ) {
		Serial.println( F("MEM SYSTEM ERROR: datablock_t size is not equal to BLOCK_SIZE") );
	}

	__ee_init(dev_addr, dev_size);
	
//	__head_db = -1;
//	__tail_db = 0;
	mem_readcounters();

	__max_db = __ee_dev_size / BLOCK_SIZE;

	__cnt_db = 0;
	
	__begin_addr = 0;
	__end_addr = (uint32_t)dev_size * 1024UL / 8;			/* this is for 24C32 memory chip, found in DS3231 breakout board */
	

#if DEBUG_MEM

#if LINUX_NATIVE_APP == 1
	fprintf(stderr, "ee_dev_size: %i\t ee_dev_addr=0x%02x\n", __ee_dev_size, __ee_dev_addr);
	mem_stats();
#else
	Serial.print( F("ee_dev_size: ") );
	Serial.print(__ee_dev_size, DEC);
	Serial.print( F("\tee_dev_addr: 0x") );
	Serial.println(__ee_dev_addr, HEX);

	mem_stats();
#endif	/* LINUX_NATIVE_APP */

#endif	/* DEBUG_MEM */
}

void mem_end()
{
	__ee_end();

	__cnt_db = 0;
}


#if RTC_CLOCK_CHIP == 1
#  define RTC_COUNTER_ADDRESS	0x7
#elif RTC_CLOCK_CHIP == 2
#  define RTC_COUNTER_ADDRESS	0x8
#else
#error Please define RTC_CLOCK_CHIP in bms.h
#endif


/* store counter __head_db and __tail_db in specific memory areas,
 * these could be the RTC special purpose memory, or other memory */
void mem_storecounters()
{

	Wire.beginTransmission( RTC_I2C_ADDRESS );
	Wire.write( RTC_COUNTER_ADDRESS );
	Wire.write( __head_db & 0xff );
	Wire.write( (__head_db >> 8) & 0xff );
	Wire.write( __tail_db & 0xff );
	Wire.write( (__tail_db >> 8) & 0xff );
	Wire.endTransmission();
}


/* read counters __head_db and __tail_db from special purpose memory */
void mem_readcounters()
{
	Wire.beginTransmission( RTC_I2C_ADDRESS );
	Wire.write( RTC_COUNTER_ADDRESS );
	Wire.endTransmission();
	Wire.requestFrom( RTC_I2C_ADDRESS, 4);
	
	__head_db = (Wire.read() & 0xff);
	__head_db |= (Wire.read() << 8) & 0xff;
	
	__tail_db = (Wire.read() & 0xff);
	__tail_db |= (Wire.read() << 8) & 0xff;
	
	
	__cnt_db = (__max_db + (__head_db - __tail_db)) % __max_db;
	
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
#if DEBUG_MEM

#if LINUX_NATIVE_APP == 1
	fprintf(stderr, "mem_write (blockno: %i, linaddr=%i)\n", blockno, __block2linearaddress(blockno));
#else
	Serial.print( F("mem_write (blockno: ") );
	Serial.print( blockno, DEC );
	Serial.print( F("\tlinadd= ") );
	Serial.print( __block2linearaddress(blockno) );
	Serial.println( F(")") );
#endif

#endif	/* DEBUG_MEM */

	__ee_writeblock( __block2linearaddress( blockno ), (char *)p, psize );
	return true;
}

bool mem_read(const void *p, uint8_t psize, uint16_t blockno)
{

#if DEBUG_MEM

#if LINUX_NATIVE_APP == 1
	fprintf(stderr, "mem_read (blockno: %i, linaddr=%i)\n", blockno, __block2linearaddress( blockno ));
#else
	Serial.print( F("mem_read (blockno: ") );
	Serial.print( blockno, DEC );
	Serial.print( F("\tlinadd= ") );
	Serial.print( __block2linearaddress(blockno) );
	Serial.println( F(")") );
#endif

#endif	/* DEBUG_MEM */

	if( __ee_readblock( __block2linearaddress( blockno ), (char *)p, psize ) == psize)
		return true;
	else
		return false;
}

bool mem_pushDatablock(datablock_t *db)
{
#if DEBUG_MEM

#if defined(LINUX_NATIVE_APP)
	fprintf(stderr, "mem_pushDatablock\n");
#else	
	Serial.println( F("mem_pushDatablock") );
#endif

#endif	/* DEBUG_MEM */

	powerRTC(1, 10);
	mem_readcounters();
	
	if(__cnt_db >= __max_db)return false;
	if((__head_db == (counter_type)-1) && (__cnt_db == 0)) {
		__head_db = 0;
		__tail_db = 0;
	}

	mem_write(db, sizeof( datablock_t ), __head_db );

//	if(__cnt_db == 0)__tail_db = __head_db;
	__cnt_db++;
	
	/* increase __head_db taking care of overflow */
	__head_db = (__head_db + 1) % __max_db;
	
	
	mem_storecounters();
	powerRTC(0, 1);
	
#if DEBUG_MEM

#if LINUX_NATIVE_APP == 1
	fprintf(stderr, "push: new __head_db: %i\n", __head_db);
#else
	Serial.print( F("push: new __head_db: ") );
	Serial.println( __head_db, DEC );
#endif

#endif	/* DEBUG_MEM */
	
	return true;
}

bool mem_popDatablock(datablock_t *db)
{
#if defined(LINUX_NATIVE_APP)
	fprintf(stderr, "mem_popDatablock\n");
#else
	Serial.println( F("mem_popDatablock") );
#endif

	powerRTC(1, 10);
	mem_readcounters();

	if(!__cnt_db)return false;
	
	mem_read(db, sizeof( datablock_t ), __tail_db);

	__tail_db = (__tail_db + 1) % __max_db;
	__cnt_db--;


	mem_storecounters();
	powerRTC(0, 1);

#if DEBUG_MEM

#if LINUX_NATIVE_APP == 1
	fprintf(stderr, "pop: new __tail_db: %i\n", __tail_db );
#else
	Serial.print( F("pop: new __tail_db: ") );
	Serial.println( __tail_db, DEC );
#endif

#endif	/* DEBUG_MEM */

	return true;
}


void datetime2db(datetime_t *dt, datablock_t *db)
{
	db->year = dt->year%100;
	db->month = dt->month;
	db->dayOfMonth = dt->dayOfMonth;
	db->hour = dt->hour;
	db->minute = dt->minute;
}

void dumpDBrecord(datablock_t *db)
{
	Serial.print(F("typ:"));
	switch(db->entryType) {
		case ENTRY_DATA: Serial.print(F("dat\t")); break;
		case ENTRY_GSM: Serial.print(F("gsm\t")); break;
		case ENTRY_GPS: Serial.print(F("gps\t")); break;
		case ENTRY_ERROR: Serial.print(F("err\t")); break;
		default:
			Serial.print(F("unk\t")); break;
	}

	Serial.print(F("nid:"));Serial.print( db->nodeId );
	Serial.print(F("\tdate:"));Serial.print( db->dayOfMonth );
	Serial.print(F("-"));Serial.print(db->month);
	Serial.print(F("-"));Serial.print(db->year);
	Serial.print(F(" "));Serial.print(db->hour);
	Serial.print(F(":"));Serial.print(db->minute);

	Serial.print(F("nid:"));Serial.print( db->nodeId );
	
	switch(db->entryType) {
		case ENTRY_DATA:
			Serial.print(F("\tVcc:"));Serial.print(db->batVolt);
			Serial.print(F("\tTem:"));Serial.print(db->bhvTemp);
			Serial.print(F("\tHum:"));Serial.print(db->bhvHumid);
			Serial.print(F("\tWei:"));Serial.println(db->bhvWeight);
			break;
		
		case ENTRY_GSM:
			break;
		
		case ENTRY_GPS:
			break;
		
		default:
			break;
	}

	Serial.println(F(";"));
}
			