
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char filetemp[128];
char *filebuf;
uint32_t filesize;
FILE *lfp;

void __ee_init(uint8_t addr, uint16_t dsize)
{
		sprintf(filetemp, "linux-file-system-0x%02x-%ukb.img", addr, dsize);

		fprintf(stderr, "ee_linux.hpp: initializing filesystem with image: %s\n", filetemp );
		lfp=fopen(filetemp, "a+");
		if(!lfp) {
			fprintf(stderr, "could not open/create filesystem image (%s)\n", filetemp);
			exit(-1);
		}
	
	__ee_dev_addr = addr;
	__ee_dev_size = dsize * 128UL;	// 128 = 1024UL / 8;

	filebuf = (char *)calloc(1, __ee_dev_size );
	filesize = __ee_dev_size;
	fread(filebuf, filesize, 1, lfp);
	fclose( lfp );
}

void __ee_end()
{
	lfp=fopen(filetemp, "w");
	fwrite(filebuf, filesize, 1, lfp);
	fclose( lfp );
}

 
void __ee_write(uint16_t addr, uint8_t data )
{
	fprintf(stderr, "__ee_write (addr:%i):", addr); 

//	fseek(lfp, addr, SEEK_SET);
//	fputc(data, lfp);
	filebuf[ addr ] = data;
}

uint8_t __ee_read(uint16_t addr )
{
	fprintf(stderr, "__ee_read (addr:%i):", addr); 

	//fseek(lfp, addr, SEEK_SET);
	
//	return( fgetc( lfp ) );

	return( filebuf[ addr ] );
}


uint8_t __ee_readblock(uint16_t addr, char *data, uint8_t datalen)
{
	fprintf(stderr, "__ee_readblock (addr:%i):", addr); 
	
//	fseek( lfp, addr, SEEK_SET );
//	i = fread(data, datalen, 1, lfp );
	memcpy(data, filebuf+addr, datalen);

  return (datalen);
}


void __ee_writeblock(uint16_t addr, char *data, uint8_t datalen)
{
	fprintf(stderr, "__ee_writeblock (addr:%i):", addr); 

//	fseek( lfp, addr, SEEK_SET );
//	fwrite(data, datalen, 1, lfp );
	memcpy(filebuf+addr, data, datalen);
}
