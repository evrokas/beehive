#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define LINUX_NATIVE_APP	1

#include "../bms.h"
#include "../data.h"

#define TEMP_BUF_LEN	32
#define	DEF_CLEAR_TEMPBUF	char _tempbuf[ TEMP_BUF_LEN ]; memset( _tempbuf, 0, TEMP_BUF_LEN )


bool gsm_getRegistration(uint8_t &areg)
{
	char *c;
  DEF_CLEAR_TEMPBUF;

//  	gsm_sendcmd("AT+CREG?\n\r");
//  	READGSM( 2 );
		strcpy( _tempbuf, "+CREG: 0,1\r\n");
		 	
  	c = strstr(_tempbuf, "+CREG: ");
  	if(c) {
  		c += 7;
  		while( *c != ',') c++;
  		c++;
  		areg = 0;
  		areg = atoi( c );
#if 1
			printf("CREG network registration: %d\n", areg);
//  		D("CREG network registration: "); Dln( areg );
#endif
			return true;
		}
		
	return false;
}

uint8_t	gsm_getBearerStatus()
{
	char *c;
  DEF_CLEAR_TEMPBUF;
  

//    gsmserial.print( F("AT+SAPBR=2,1\r\n") );
//    gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );
		strcpy(_tempbuf, "+SAPBR: 0,1,100.32.12.99\r\n");


    c = strstr(_tempbuf, "+SAPBR: ");
    if(!c)return 0xff;
    c += 8;
    c = strchr( c, ',' );
    c++;
    
    /* SIM800 manual
     * response is: +SAPBR: <cid>,<status>,<IP_Addr>
     * 		<cid>	bearer profile identifier
     *		<status>	0	bearer is connecting
     *					1	bearer is connected
     *					2	bearer is closing
     *					3	bearer is closed
     */
     
    switch( *c ) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        default: return 0xff;
    }
}



int main(int argc, char *argv[])
{
  uint8_t a;
  
	printf("Utility program\n");
	
	printf("size of datablock_t structure: %lu\n", sizeof( datablock_t ) );

	gsm_getRegistration( a );
	printf("Bearer status: %d\n", gsm_getBearerStatus() );
  return 0;
}
