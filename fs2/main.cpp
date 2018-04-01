#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../bms.h"
#include "data.h"
#include "../mem.h"

SECTOR	sect;

void printSector(SECTOR *sct)
{
  int i;

	printf("t: %c %c\t", DECODE_TYPE(sct->header)&0x2?'1':'0', DECODE_TYPE(sct->header)&0x1?'1':'0');
	printf("head: 0x%05x (%u)\ttail: 0x%05x (%u)\n", 
		DECODE_HEAD(sct->header), DECODE_HEAD(sct->header), 
		DECODE_TAIL(sct->header), DECODE_TAIL(sct->header));

	for(i=0;i<SECTOR_HEADER_SIZE;i++) {
		printf("%02x ", sct->header[i]);
	}
	putc('\n', stdout);


	for(i=0;i<SECTOR_PAYLOAD_SIZE;i++) {
		printf("%02x ", sct->payload[i]);
		if(i % 16 == 15)putc('\n', stdout);
	}
	putc('\n', stdout);
	
}

void test_header(void)
{
	ENCODE_TYPE(sect.header, SECTOR_BAD);
	ENCODE_HEAD(sect.header, 0x11);
	ENCODE_TAIL(sect.header, 0x20);

	printSector(&sect);
	
	printf("Setting HEAD to 0x12\n");
	ENCODE_HEAD(sect.header, 0x12);
	printSector(&sect);
	
	printf("Setting TAIL to 0x30\n");
	ENCODE_TAIL(sect.header, 0x30);
	printSector(&sect);

	printf("Setting HEAD to 8191\n");
	ENCODE_HEAD(sect.header, 8191);
	printSector(&sect);
	
	printf("Setting TAIL to 8191\n");
	ENCODE_TAIL(sect.header, 8191);
	printSector(&sect);
}

void pdb(datablock_t *d)
{
	printf("nodeId: %04d batVolt: %03d temp: %05d humid: %05d timestamp: %08d weight: %05d\n",
		d->nodeId, d->batVolt, d->bhvTemp, d->bhvHumid, d->rtcDateTime, d->bhvWeight);
}


int main(int argc, char *argv[])
{
	datablock_t dt;

	printf("Utility program\n");
	
	printf("size of datablock_t structure: %lu\n", sizeof( datablock_t ) );

	printf("%s %s\n", __DATE__, __TIME__ );
	

	mem_init( 32, 0xfa );
	
	
	dt.nodeId = 1000;
	dt.batVolt = 4134;
	dt.bhvTemp = 1913;
	dt.bhvHumid = 4014;
	dt.rtcDateTime = 0;
	dt.bhvWeight = 7654;
	
	mem_pushDatablock( &dt );
	mem_stats();
	
	dt.nodeId = 2000;
	mem_pushDatablock( &dt ); 
	
	mem_stats();

	dt.nodeId = 3000;
	mem_pushDatablock( &dt );
	
	mem_stats();
	
	mem_popDatablock( &dt );
	pdb( &dt );
	fprintf(stderr, "node = %i\n", dt.nodeId );
	
 	mem_stats();

	mem_popDatablock( &dt );
	pdb( &dt );
	fprintf(stderr, "node = %i\n", dt.nodeId );
	
 	mem_stats();

	mem_popDatablock( &dt );
	pdb( &dt );
	fprintf(stderr, "node = %i\n", dt.nodeId );
	
 	mem_stats();

	mem_popDatablock( &dt );
	pdb( &dt );
	fprintf(stderr, "node = %i\n", dt.nodeId );
	
	mem_stats();


 	dt.nodeId = 2002;
	mem_pushDatablock( &dt ); 
	
	mem_stats();

	{ 
		datablock_t dt2;
		mem_popDatablock( &dt2 );
		pdb( &dt2 );
		fprintf(stderr, "dt2: node = %i\n", dt2.nodeId );
		
		mem_stats();
	}
	
	
	mem_end();
		
  return 0;
}
