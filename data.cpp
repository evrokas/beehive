
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "data.h"
#include "mem.h"

//#define FILESYSTEM_NAME	"filesystem"
//FILE *fd;

#define STATSYSTEM_NAME	"filesystem.stats"
FILE *stfd;


RING fstypes[ SECTOR_TYPE_COUNT ];

STATS *strecs;

//#define EEPROM_SIZE		512
#define EEPROM_SIZE		32
#define BANK_COUNT		(1)
#define BIT_COUNT			(BANK_COUNT * EEPROM_SIZE * 1024)
#define BYTE_COUNT		(BIT_COUNT>>3)
#define SECTOR_COUNT	(BYTE_COUNT>>5)


void __fs_readSector(uint16_t sectorIndex, void *buf)
{
	__ee_readblock( sectorIndex * sizeof( SECTOR ), (char *)buf, sizeof( SECTOR ) );

#if 0
	fseek(fd, sectorIndex * sizeof( SECTOR ), SEEK_SET);

	if(fread(buf, 1, sizeof( SECTOR ), fd) != sizeof( SECTOR )) {
		fprintf(stderr, "%s: error reading correct number of bytes from filesystem\n", "fs_readsector");
		exit(-1);
	}
#endif
	
	strecs[ sectorIndex ].rdCount++;
}

void __fs_writeSector(uint16_t sectorIndex, void *buf)
{
#if 0
	printf("__fs__writeSector: writing sector: %u (pos: %lu)\n", sectorIndex, sectorIndex * sizeof( SECTOR ));
#endif

	__ee_writeblock( sectorIndex * sizeof( SECTOR ), (char *)buf, sizeof( SECTOR ) );

#if 0
	fseek(fd, sectorIndex * sizeof( SECTOR ), SEEK_SET);
	if((i=fwrite(buf, 1, sizeof( SECTOR ), fd)) != sizeof( SECTOR )) {
		fprintf(stderr, "%s: error writing correct number of bytes to filesystem (%u)\n", "fs_writesector", i);
		exit(-1);
	}
#endif

	strecs[ sectorIndex ].wrCount++;
}

void fs_writeSector(SECTOR *buf)
{
  uint8_t n;

		n = DECODE_TYPE( buf->header );
		switch( n ) {
			case SECTOR_DATA:
							fstypes[n].HEAD++;
							ENCODE_HEAD( buf->header, fstypes[n].HEAD );
							ENCODE_TAIL( buf->header, fstypes[n].TAIL );
							ENCODE_TYPE( buf->header, SECTOR_DATA );

							fprintf(stderr, "sector_data\th: %u\tt: %u\n", fstypes[n].HEAD, fstypes[n].TAIL);

							__fs_writeSector( fstypes[n].HEAD, (void *)buf );
							break;
			case SECTOR_CONFIG:
							fstypes[n].HEAD++;
							ENCODE_HEAD( buf->header, fstypes[n].HEAD );
							ENCODE_TAIL( buf->header, fstypes[n].TAIL );
							ENCODE_TYPE( buf->header, SECTOR_CONFIG );
							__fs_writeSector( fstypes[n].HEAD, (void *)buf );
							break;
			case SECTOR_BAD: break;
			default: ;
		}
}

void filesystem_init()
{
  unsigned int i, n;
  uint16_t ahead=0xffff, atail=0xffff;
	SECTOR s;
	  

		printf("Initialize file system\n");
		printf("%u bits\t%u bytes\t%u sector(s)\n", BIT_COUNT, BYTE_COUNT, SECTOR_COUNT);

		
		/* last type is SECTOR_BAD, do not need to initialize this */
		for(i=0;i<SECTOR_TYPE_COUNT-1;i++) {
			fstypes[i].HEAD = 0xffff;
			fstypes[i].TAIL = 0xffff;
		}

#if 0
		fd = fopen(FILESYSTEM_NAME, "r+");
		if(!fd) {
			fprintf(stderr, "could not open filesystem file: %s\n", FILESYSTEM_NAME);
			exit(-1);
		}
#else
		__ee_init( 0xfa, 32 );
#endif
		
		stfd = fopen(STATSYSTEM_NAME, "r+");
		if(!stfd) {
			fprintf(stderr, "could not open stats file: %s\n", STATSYSTEM_NAME);
			exit(-1);
		}

		printf("stats file system size: %lu\n", sizeof( STATS ) * SECTOR_COUNT );
		strecs = (STATS *)calloc( SECTOR_COUNT, sizeof( STATS ));
		i=fread(strecs, sizeof( STATS ), SECTOR_COUNT, stfd);
		if(i != SECTOR_COUNT) {
			fprintf(stderr, "wrong STATS filesystem size (%u)\n", i);
			exit(-1);
		}

		//dumpStats();

		printf("Scanning filesystem ...");
		for(i=0;i<SECTOR_COUNT;i++) {
			__fs_readSector(i, (void *)&s);

#if 0
			printf("%04u ", i);
			if(i%16==0)putc('\n', stdout);
#endif
			
			ahead = DECODE_HEAD(s.header);
			atail = DECODE_TAIL(s.header);
			
			n = DECODE_TYPE(s.header);
			if(n == SECTOR_BAD)continue;


			if((fstypes[n].HEAD == 0xffff) || (fstypes[n].TAIL == 0xffff)) {
				if(fstypes[n].HEAD == 0xffff)fstypes[n].HEAD = ahead;
				if(fstypes[n].TAIL == 0xffff)fstypes[n].TAIL = atail;
			} else {
				if(ahead > fstypes[n].HEAD)fstypes[n].HEAD = ahead;
				if(atail > fstypes[n].TAIL)fstypes[n].TAIL = atail;
			}
		}
		printf("OK\n");

		printf("#\tHEAD\tTAIL\n");
		for(i=0;i<SECTOR_TYPE_COUNT-1;i++)
			printf("[%u]: %04u\t%04u\n", i, fstypes[i].HEAD, fstypes[i].TAIL);
}

void filesystem_done()
{
	unsigned int i;


		fseek(stfd, 0, SEEK_SET);
		i=fwrite(strecs, sizeof( STATS ), SECTOR_COUNT, stfd);
		if(i != SECTOR_COUNT) {
			fprintf(stderr, "error writing correct number of bytes in STATS file system\n");
		}
		
		fclose(stfd);
		
#if 0
		fclose(fd);
#else
		__ee_end();
#endif

}

void dumpStats()
{
  uint16_t i;

  	printf("---- dumpStats() ----\n");
		for(i=0;i<SECTOR_COUNT;i++) {
#if 0
			if(strecs[i].rdCount)
				printf("[%04u] rd: %u\n", i, strecs[i].rdCount);
#endif
			if(strecs[i].wrCount)
				printf("[%04u] wr: %u\n", i, strecs[i].wrCount);
		}
		printf("----- END ----\n");
}
