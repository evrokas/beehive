/*
 * mem.h - memory module header
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-17
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __MEM_H__
#define __MEM_H__

#include "bms.h"
#include "data.h"

#define BLOCK_SIZE	16


#ifdef __cplusplus
extern "C" {
#endif

extern int32_t	__head_db;		/* head pointer of datablocks */
extern int32_t	__tail_db;		/* tail pointer of datablocks */
extern int32_t	__max_db;			/* maximum data block available */

extern int32_t	__begin_addr;	/* first EEPROM address to access */
extern int32_t	__end_addr;		/* last EEPROM address to access */

void __ee_write(uint16_t addr, uint8_t data );
uint8_t __ee_read(uint16_t addr ); 
uint8_t __ee_readblock(uint16_t addr, char *data, uint8_t datalen);
void __ee_writeblock(uint16_t addr, char *data, uint8_t datalen);
 
void __ee_init(uint8_t addr, uint16_t dsize);
void __ee_end();

void mem_init(uint32_t dev_size, uint8_t dev_addr);
void mem_end();

void mem_stats();

bool mem_write(const void *p, uint8_t psize);
bool mem_read(const void *p, uint8_t psize);

bool mem_pushDatablock(datablock_t *db);

bool mem_popDatablock(datablock_t *db);
	



#ifdef __cplusplus
}	/* extern "C" */
#endif

#endif	/* __MEM_H__ */
