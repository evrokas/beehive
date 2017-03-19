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
#define	DEF_CLEAR_TEMPBUF	char _tempbuf[ TEMP_BUF_LEN ]; memset( _tempbuf, 0, TEMP_BUF_LEN )

/* shortcut to read data in _tempbuf */
#define READGSM( lat )	gsm_readSerial( _tempbuf, TEMP_BUF_LEN, (lat) )

#define CF(str)	(char *)( str )


SoftwareSerial gsmserial(GSM_RX, GSM_TX);

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
	gsmserial.begin( GSM_SERIAL_BAUDRATE );
}

bool gsm_sendrecvcmd(char *cmd, char *expstr)
{
	DEF_CLEAR_TEMPBUF;
	  
		gsmserial.print( cmd );
    
		if( READGSM( 2 ) ) {
			/* there was a string response read in 2sec */
			if(strstr(_tempbuf, expstr)) {
				/* expected string found */
				return true;
			} else return false;
		} else return false;
}

bool gsm_sendrecvcmdtimeout(char *cmd, char *expstr, uint8_t timeout)
{
	DEF_CLEAR_TEMPBUF;
  
		gsmserial.print( cmd );
    
		if( READGSM ( timeout ) ) {
			/* there was a string response read in timeout secs */
			if(strstr(_tempbuf, expstr)) {
				/* expected string found */
				return true;
			} else return false;
		} else return false;
}

void gsm_sendcmd(char *cmd)
{
	gsmserial.print( cmd );
}

void gsm_relayOutput( Stream &ast )
{
	while( gsmserial.available() )ast.write( gsmserial.read() );
}




bool gsm_activateBearerProfile(char *apn, char *user, char *pass)
{
	char _tempbuf[ TEMP_BUF_LEN ];
//AT+SAPBR=3,1,"CONTYPE","GPRS"
//AT+SAPBR=3,1,"APN","myq"
//AT+SAPBR=3,1,"USER",""
//AT+SAPBR=3,1,"PWD",""

		/* set parameters */
    if(!gsm_sendrecvcmdtimeout( CF( "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n") , CF( "OK\r\n") , 2 ) )
    	return false;
    
    	
    gsmserial.print( CF( F("AT+SAPBR=3,1,\"APN\",\"") ) );
    	gsmserial.print( apn );
    	gsmserial.print("\"\r\n");

		if(!READGSM( 2 ) || !strstr(_tempbuf, CF( F("OK\r\n") ) ))return false;


    gsmserial.print( F("AT+SAPBR=3,1,\"USER\",\"") );
        gsmserial.print( user );
        gsmserial.print("\"\r\n");
    if(!READGSM( 2 ) || !strstr(_tempbuf, "OK\r\n"))return false;

    gsmserial.print( F("AT+SAPBR=3,1,\"PWD\",\"") );
        gsmserial.print( pass );
        gsmserial.print( F("\"\r\n") );
    if(!READGSM( 2 ) || !strstr(_tempbuf, "OK\r\n"))return false;

    
    /* actual connection */
    if(!gsm_sendrecvcmdtimeout(CF( F("AT+SAPBR=1,1\r\n") ), CF( F("OK\r\n") ), 85) )
    return false;

    /* so we are connected! */
  return true;
}

uint8_t	gsm_getBearerStatus()
{
	char *c;
  DEF_CLEAR_TEMPBUF;
  

    gsm_sendcmd( CF( F("AT+SAPBR=2,1\r\n") ) );
    READGSM( 2 );

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


bool gsm_deactivateBearerProfile()
{
	DEF_CLEAR_TEMPBUF;

		if(!gsm_sendrecvcmdtimeout( CF( F("AT_SAPBR=0,1\r\n") ), CF( F( "OK\r\n" ) ), 65 ) )
			return false;

  return true;
}

bool http_getRequest(char *url, char *args, uint16_t &datalen)
{
	char *c;
	DEF_CLEAR_TEMPBUF;
	uint16_t result;

		if(!gsm_sendrecvcmdtimeout( CF( F("AT+HTTPINIT\r\n") ), CF( F( "OK\r\n" ) ), 2 ) )
			return false;

		if(!gsm_sendrecvcmdtimeout( CF( F("AT+HTTPPARA=\"CID\",1\r\n") ), CF( F( "OK\r\n" ) ), 2 ) ) {
			gsm_sendcmd( CF( F("AT+HTTPTERM\r\n") ) );
			return false;
		}
		
		gsm_sendcmd( CF( F("AT+HTTPPARA=\"URL\",\"") ) );
		gsm_sendcmd( url );
		gsm_sendcmd( CF( F("\",\"") ) );
		gsm_sendcmd( args );
		gsm_sendcmd( CF( F("\"\r\n") ) );
		
		READGSM( 2 );
		if(!strstr(_tempbuf, CF( F( "OK\r\n") ) ) ) {
			gsm_sendcmd( CF( F("AT+HTTPTERM\r\n") ) );
			return false;
		}
		
		gsm_sendcmd( CF( F("AT_HTTPACTION=0\r\n") ) );
		if( !READGSM( 15 ) ) {
			gsm_sendcmd( CF( F("AT+HTTPTERM\r\n") ) );
			return false;
		}

		/* SIM800 manual:
		 * response is:
		 * +HTTPACTION: <Method>,<StatusCode>,<DataLen>
		 * Parameter <Method> HTTP method specification:
		 * 0 GET
		 * 1 POST
		 * 2 HEAD
		 * 
		 * <StatusCode> HTTP Status Code responded by remote server
		 * 100 Continue
		 * 101 Switching Protocols
		 * 200 OK
		 *
		 * <DataLen>	the length of data got
		 */
		 
		c = strstr(_tempbuf, "+HTTPACTION");
		if(!c) {
			gsm_sendcmd( CF( F("AT+HTTPTERM\r\n") ) );
			return false;
		}
		       
		c = strchr(c, ',') + 1;

		/* only accept 200 (OK) request, all other are a fail so it must be
		 * treated as such */
		c = strstr(c, "200");
		if(!c) {
			gsm_sendcmd( CF( F("AT+HTTPTERM\r\n") ) );
			return false;
		}
		
		/* valid responses, are usual HTTP reponses: (see manual for more codes)
		 * 200	OK
		 * 404	NOT FOUND
		 * 601	NETWORK ERROR
		 * 602	NO MEMORY
		 * 603	DNS ERROR
		 */
		c = strchr(c, ',');
		if(!c) {
			gsm_sendcmd( CF( F("AT+HTTPTERM\r\n") ) );
			return false;
		}
		
		datalen = atoi( ++c );
		
		gsmserial.print( CF( F("AT+HTTPREAD\r\n") ) );
		gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
				 
		/* clean receive buffer */
		while(gsmserial.available())gsmserial.read();
			
	return (result);
}


bool gsm_moduleInfo()
{
	DEF_CLEAR_TEMPBUF;
	
  	gsm_flushInput();

  	gsmserial.print( CF( F("ATI\r\n") ) );
  	gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );
  	Serial.print("GSM response: <"); Serial.print( _tempbuf ); Serial.println(">");
  	
  	_tempbuf[12] = '\0';
  	Serial.print(">>"); Serial.print(_tempbuf+6); Serial.println("<<");
  	
  	if(!strncmp( _tempbuf+6, "SIM800", 6 ))return true;
  	else return false;
}

bool gsm_getBattery(uint16_t &bat)
{
	char *c;
	DEF_CLEAR_TEMPBUF;

		gsm_flushInput();
		gsmserial.print( CF( F("AT+CBC\r\n") ) );
		if( !READGSM( 2 ) )return false;
		
		c = strstr( _tempbuf, "CBC: ");
		if(!c)return false;
		
		c = strrchr( c, ',' );
		if(!c)return false;
		
		bat = atoi( ++c );

#if 0
  	Serial.print("CBC battery level: "); Serial.println( bat );
#endif

	return true;
}


bool gsm_available()
{
	return (gsmserial.available());
}


char gsm_read()
{
	return (gsmserial.read());
}


void gsm_write(char c)
{
	gsmserial.write(c);
}

void gsm_flushInput()
{
	while( gsmserial.available() )
		gsmserial.read();
}



bool gsm_sendPin(char *aspin)
{
	DEF_CLEAR_TEMPBUF;

		gsm_flushInput();
		gsmserial.print( CF( F("AT+CPIN=") ) );
		gsmserial.print( aspin );
		gsmserial.println(CF( F("\r\n") ) );
		gsm_readSerial( _tempbuf, TEMP_BUF_LEN, 2 );
		if(!strstr(_tempbuf, CF( F("OK\r\n") ) )) {
			Serial.println(CF( F( "Error setting cpin\n" ) ) );
			return false;
		}
		
	return true;
}


bool gsm_moduleReady()
{
	char *c;
	DEF_CLEAR_TEMPBUF;

	if( !gsm_sendrecvcmdtimeout( CF( F("AT\n\r") ) , CF( F("OK\r\n") ), 2) )return false;
	gsm_sendcmd( CF( F("AT+CREG?\n\r") )  );
	if( !READGSM( 5 ) )return false;
	c = strstr( _tempbuf, CF( F("+CREG: ") ) );
	if(!c)return false;
	
	c = strchr( c, ',' );
	if(!c)return false;

	switch( *( ++c ) ) {
		case '1':
		case '5':
			return true;
		default:
			return false;
	}
}


bool gsm_getRegistration(uint8_t &areg)
{
	char *c;
  DEF_CLEAR_TEMPBUF;

  	gsm_sendcmd( CF( F("AT+CREG?\n\r" ) ) );
  	READGSM( 2 );
  	
  	c = strstr(_tempbuf, CF( F("+CREG: ") ) );
  	if(!c)return false;
  	
  	c = strchr(c, ',');
  	areg = atoi( ++c );
#if 0
  		D("CREG network registration: "); Dln( areg );
#endif
	
	return true;
}


bool gsm_moduleLowPower( bool alowpower )
{
	if( alowpower ) {
		if( gsm_sendrecvcmdtimeout( CF( F("AT+CFUNC=0\n\r") ) , CF( F("OK\r\n") ), 2) )return true;
		else return false;
	} else {
		if( gsm_sendrecvcmdtimeout("AT+CFUNC=1\n\r", "OK\r\n", 2) )return true;
		else return false;
	}
}

bool gsm_getSignalQuality(uint8_t &asqual)
{
	char *c;
  DEF_CLEAR_TEMPBUF;

  	gsm_sendcmd( CF( F( "AT+CSQ\n\r" ) ) );
  	READGSM( 2 );
  	
  	c = strstr(_tempbuf, CF( F( "CSQ: " ) ) );
  	if(c) {
  		c += 5;
  		asqual = atoi( c );
  		
#if 0
  		D("CSQ signal quality: "); Dln( asqual );
#endif
			return true;
		}
		
	return false;
}

bool gsm_getDateTimeLonLat(uint8_t &hour, uint8_t &min, uint8_t &sec, uint8_t &day, uint8_t &month,
uint8_t &year, float &lon, float &lat)
{
	char *c;
	int16_t res;
	DEF_CLEAR_TEMPBUF;

		gsm_sendcmd("AT+CIPGSMLOC=1,1\n");
		
		c=strstr( _tempbuf, "GSMLOC: " );
		if(!c)return false;
	
		/* reponse is:
		 * +CIPGSMLOC:<locationcode>[,<longitude>,<latitude>,<date>,<time>]
		 * where:
		 *  <locationcode> is 0 if command succesful, or i.e. 404 NOT FOUND
		 *  <longitude>
		 *  <latitude>
		 *  <date>	YYYY/MM/DD
		 *  <time>	hh/mm/ss
		 */
		 
		//<locationcode>
		res = atoi( ++c );
		if(res != 0)return false;
		
		// longitude
		c=strchr(c, ',');	
		if(!c)return false;
		lon = atof( ++c );
		
		// latitude
		c=strchr(c, ',');
		if(!c)return false;
		lat = atof( ++c );
		
		// year YYYY
		c=strchr(c, ',');
		if(!c)return false;
		year = atoi( ++c ) - 2000;
		
		// month MM
		c=strchr(c, '/');
		if(!c)return false;
		month = atoi( ++c );
		
		// day DD
		c=strchr(c, '/');
		if(!c)return false;
		day = atoi( ++c );
		
		// hour hh
		c=strchr(c, ',');
		if(!c)return false;
		hour = atoi( ++c );
		
		// minute mm
		c=strchr(c, ':');
		if(!c)return false;
		min = atoi( ++c );
		
		// seconds ss
		c=strchr(c, ':');
		if(!c)return false;
		sec = atoi( ++c );
		
	return true;
}
