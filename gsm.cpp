/*
 * gsm.cpp - GSM/GPRS Module functions file
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-17
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */


#include <Arduino.h>
#include <SoftwareSerial.h>
#include "gsm.h"


#define TEMP_BUF_LEN	32


SoftwareSerial gsmserial(GSM_RX, GSM_TX);
//char _tempbuf[ TEMP_BUF_SIZE ], *tch;

/* buf is buffer to store contents, buflen is length of buffer, timeout in seconds */
uint8_t gsm_readSerial(char *buf, uint8_t buflen, uint8_t timeout)
{
    while(timeout && !gsmserial.available()) {
        delay(800);
        timeout -= 1;
    }
    
    if(gsmserial.available()) {
        
        /* while still room in buf and available characters from serial */
        while(buflen && gsmserial.available()) {
            *buf++ = gsmserial.read();
            buflen--;
        }
        
        /* return with current buf */
        return true;
    } else
        return false;	/* return no read */
}


void gsm_init()
{
    gsmserial.begin( GSM_DEFAULT_BAUDRATE );
}

bool gsm_sendrecvcmd(char *cmd, char *expstr)
{
  char _tempbuf[ TEMP_BUF_LEN ];
  
    gsmserial.print( cmd );
    
    if(gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2)) {
        /* there was a string response read in 2sec */
        if(strstr(_tempbuf, expstr)) {
            /* expected string found */
            return 1;
        } else return 0;
    } else return 0;
}


bool gsm_activateBearerProfile(char *apn, char *user, char *pass)
{
    char _tempbuf[ TEMP_BUF_LEN ];
//AT+SAPBR=3,1,"CONTYPE","GPRS"
//AT+SAPBR=3,1,"APN","myq"
//AT+SAPBR=3,1,"USER",""
//AT+SAPBR=3,1,"PWD",""

    /* set parameters */
    gsmserial.print( F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n") );
    gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
    if(!strstr(_tempbuf, "OK\r\n"))return false;

    gsmserial.print( F("AT+SAPBR=3,1,\"APN\",\"") );
        gsmserial.print( apn );
        gsmserial.print("\"\r\n");
    gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
    if(!strstr(_tempbuf, "OK\r\n"))return false;


    gsmserial.print( F("AT+SAPBR=3,1,\"USER\",\"") );
        gsmserial.print( user );
        gsmserial.print("\"\r\n");
    gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
    if(!strstr(_tempbuf, "OK\r\n"))return false;

    gsmserial.print( F("AT+SAPBR=3,1,\"PWD\",\"") );
        gsmserial.print( pass );
        gsmserial.print( F("\"\r\n") );
    gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
    if(!strstr(_tempbuf, "OK\r\n"))return false;

    
    /* actual connection */
    gsmserial.print( F("AT+SAPBR=1,1\r\n") );
    gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 85 );		/* for timeout see SIM800 command manual */
    if(!strstr(_tempbuf, "OK\r\n"))return false;


    /* so we are connected! */
  return true;
}

uint8_t	gsm_getBearerStatus()
{
  char _tempbuf[ TEMP_BUF_LEN ];
  
    gsmserial.print( F("AT+SAPBR=2,1\r\n") );
    gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );
    
    /* SIM800 manual
     * response is: +SAPBR: <cid>,<status>,<IP_Addr>
     * 		<cid>	bearer profile identifier
     *		<status>	0	bearer is connecting
     *					1	bearer is connected
     *					2	bearer is closing
     *					3	bearer is closed
     */
     
    switch( *(strchr(_tempbuf, ',') + 1) ) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        default: return 0xff;
    }
}


bool gsm_deactivateBearerProfile()
{
  char _tempbuf[ TEMP_BUF_LEN ];

      gsmserial.print( F("AT_SAPBR=0,1\r\n") );
      gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 65 );
      
      if(!strstr(_tempbuf, "OK\r\n"))return false;

  return true;
}

uint16_t http_getRequest(char *url, char *args)
{
	char _tempbuf[ TEMP_BUF_LEN ], *_t;
	uint16_t result;

		gsmserial.print( F("AT+HTTPINIT\r\n") );
		gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
		if(!strstr(_tempbuf, "OK\r\n"))return false;
       
		gsmserial.print( F("AT+HTTPPARA=\"CID\",1\r\n") );
		gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
		if(!strstr(_tempbuf, "OK\r\n"))return false;
       
		gsmserial.print( F("AT+HTTPPARA=\"URL\",\"") );
		gsmserial.print( url );
			gsmserial.print( "\",\"" );
			gsmserial.print( args );
			gsmserial.print( F("\"\r\n") );

		gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
		if(!strstr(_tempbuf, "OK\r\n"))return false;

		gsmserial.print( F("AT_HTTPACTION=0\r\n") );
		gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 15 );
		_t = strstr(_tempbuf, "+HTTPACTION");
		if(!_t)return false;
       
		_t += 11;
       
		_t = strchr(_t, ',') + 1;

    result = 	(*(_t++) - '0') * 100 +
    					(*(_t++) - '0') * 10 +
    					(*_t - '0');

		/* valid responses, are usual HTTP reponses: (see manual for more codes)
		 * 200	OK
		 * 404	NOT FOUND
		 * 601	NETWORK ERROR
		 * 602	NO MEMOry
		 * 603	DNS ERROR
		 */

		gsmserial.print( F("AT+HTTPREAD\r\n") );
		gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
				 
		/* clean receive buffer */
		while(gsmserial.available())gsmserial.read();
			
	return (result);
}

