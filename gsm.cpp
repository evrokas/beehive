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
#include "gsm.h"
#include "data.h"


#define USE_NEOSW
#ifdef USE_NEOSW

#include <NeoSWSerial.h>
NeoSWSerial gsmserial(GSM_RX, GSM_TX);

#else

#include <SoftwareSerial.h>
SoftwareSerial gsmserial(GSM_RX, GSM_TX);

#endif	/* USE_NEOSW */


#define TEMP_BUF_LEN	32
#define	DEF_CLEAR_TEMPBUF	char _tempbuf[ TEMP_BUF_LEN ]; memset( _tempbuf, 0, TEMP_BUF_LEN )
#define CLEAR_TEMPBUF	memset( _tempbuf, 0, TEMP_BUF_LEN )

/* shortcut to read data in _tempbuf */
#define READGSM( lat )	gsm_readSerial( _tempbuf, TEMP_BUF_LEN, (lat) )

#define CF(str)	(char *)( str )

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
    
//    Serial.print( buf );
        
		/* return with current buf */
		return true;
	} else
		return false;	/* return no read */
}


void gsm_init()
{
	gsmserial.begin( GSM_SERIAL_BAUDRATE );
}

#if 1
bool gsm_sendrecvcmd(char *cmd, char *expstr)
{
	DEF_CLEAR_TEMPBUF;

		while(gsm_available())gsm_read();
	  
		gsm_sendcmd( cmd );
    
		if( READGSM( 2 ) ) {
			/* there was a string response read in 2sec */
			if(strstr(_tempbuf, expstr)) {
				/* expected string found */
				return true;
			} else return false;
		} else return false;
}
#endif

bool gsm_sendrecvcmdtimeout(char *cmd, char *expstr, uint8_t timeout)
{
	DEF_CLEAR_TEMPBUF;
	uint32_t mils;
	char *tb;
  
		while(gsm_available())gsm_read();

		gsm_sendcmd( cmd );

		mils = millis() + 1000UL * timeout;
		
		tb = _tempbuf;
		while( millis() < mils ) {
			while(gsm_available()) {
				if(strlen(_tempbuf) < TEMP_BUF_LEN-1) {
					*tb++ = gsm_read();
				} else {
					return false;
				}
			
				if(strstr(_tempbuf, expstr)) {
					/* expected string found */
					return true;
				}
			}
		}
		
		/* timeout */
	return false;
}


void gsm_sendcmd(char *cmd)
{
	while(gsm_available())gsm_read();
	
	gsmserial.print( cmd );
//	Serial.print( cmd );
}

void gsm_relayOutput( Stream &ast )
{
	while( gsmserial.available() )ast.write( gsmserial.read() );
}




bool gsm_activateBearerProfile(char *apn, char *user, char *pass)
{
	DEF_CLEAR_TEMPBUF;
//AT+SAPBR=3,1,"CONTYPE","GPRS"
//AT+SAPBR=3,1,"APN","myq"
//AT+SAPBR=3,1,"USER",""
//AT+SAPBR=3,1,"PWD",""

		if(!gsm_sendrecvcmdtimeout( CF("AT+CIPSHUT\r\n"), CF("SHUT OK\r\n"), 2) )
			return false;
			
		if(!gsm_sendrecvcmdtimeout( CF("AT+CIPMUX=0\r\n"), CF("OK\r\n"), 2 ) )
			return false;


		/* set parameters */
    if(!gsm_sendrecvcmdtimeout( CF( "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n") , CF( "OK\r\n") , 2 ) )
    	return false;
    
		strcpy(_tempbuf, ("AT+SAPBR=3,1,\"APN\",\"") );
		strcat(_tempbuf, apn );
		strcat(_tempbuf, ("\"\r\n") );
		if(!gsm_sendrecvcmdtimeout( CF( _tempbuf ), CF( "OK\r\n" ), 2 ) )
			return false;
		
		strcpy(_tempbuf, ("AT+SAPBR=3,1,\"USER\",\"") );
		strcat(_tempbuf, user );
		strcat(_tempbuf, "\"\r\n" );
		if(!gsm_sendrecvcmdtimeout( CF( _tempbuf ), CF( "OK" ), 2 ) )
			return false;
		
		strcpy(_tempbuf, ("AT+SAPBR=3,1,\"PWD\",\"") );
		strcat(_tempbuf, pass );
		strcat(_tempbuf, "\"\r\n" );
		if(!gsm_sendrecvcmdtimeout( CF(_tempbuf ), CF( "OK\r\n" ), 2 ) )
			return false;
			
    /* actual connection */
    if(!gsm_sendrecvcmdtimeout(CF( ("AT+SAPBR=1,1\r\n") ), CF( ("OK\r\n") ), 85) )
    return false;

    /* so we are connected! */
  return true;
}

uint8_t	gsm_getBearerStatus()
{
	char *c;
  DEF_CLEAR_TEMPBUF;
  

    gsm_sendcmd( CF( ("AT+SAPBR=2,1\r\n") ) );
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

		if(!gsm_sendrecvcmdtimeout( CF( ("AT+SAPBR=0,1\r\n") ), CF( ( "OK\r\n" ) ), 65 ) )
			return false;

  return true;
}

bool http_initiateGetRequest()
{
	DEF_CLEAR_TEMPBUF;

		if(!gsm_sendrecvcmdtimeout( CF( "AT+HTTPINIT\r\n" ), CF( "OK\r\n" ),  2 ) ) {
			return false;
		}
		
		if(!gsm_sendrecvcmdtimeout( CF( "AT+HTTPPARA=\"CID\",1\r\n" ), CF( "OK\r\n" ), 2 ) ) {
			gsm_sendcmd( CF( "AT+HTTPTERM\r\n" ) );
			return false;
		}

	return (true);
}
		
void http_terminateRequest()
{
	gsm_sendrecvcmdtimeout( CF( "AT+HTTPTERM\r\n" ) , CF( "OK\r\n" ), 2);
}



#define SERVER_URL	CF( "5.55.150.188" )
#define SERVER_PORT	CF( "8088" )


bool http_send_datablock(datablock_t &db)
{
	DEF_CLEAR_TEMPBUF;
	char	cbuf[12], *c;
	uint16_t dlen;


		Serial.println( F("trying to send data block") );
		
		gsm_sendcmd( CF( "AT+HTTPPARA=\"URL\",\"http://") );
		gsm_sendcmd( SERVER_URL );
		gsm_sendcmd( CF( ":" ) );
		gsm_sendcmd( SERVER_PORT );
		//gsm_sendcmd( CF( "\",\"" ) );
		gsm_sendcmd( CF( "/data.php?action=add" ) );
		
#define SEND(arg, fmt, value)	\
													sprintf(_tempbuf, fmt, arg, value); \
													Serial.println( _tempbuf ); \
													gsm_sendcmd( _tempbuf )

#if 0
													gsm_sendcmd( CF( "&" ) ); \
													gsm_sendcmd( CF( arg ) );	\
													gsm_sendcmd( CF( "=" ) ); \
													gsm_sendcmd( CF( value ) )
#endif

		SEND( CF("apikey"), "&%s=%s", "abcdefgh" );
		SEND( CF("nodeId"), "&%s=%d", db.nodeId );
		SEND( CF("mcuTemp"), "&%s=%d", 100 );	//db.mcuTemp );  

		c = dtostrf(((float)db.batVolt/1000.0), 0, 3, cbuf );
		SEND( CF("batVolt"), "&%s=%s", cbuf );
		
		c = dtostrf(((float)db.bhvTemp/100.0), 0, 2, cbuf );
		SEND( CF("bhvTemp"), "&%s=%s", cbuf );


		c = dtostrf(((float)db.bhvHumid/100.0), 0, 2, cbuf );
		SEND( CF("bhvHumid"), "&%s=%s", cbuf );
		
		SEND( CF("rtcDateTime"), "&%s=%s", "22-9-17_10:51" );
		SEND( CF("gsmSig"), "&%s=%d", db.gsmSig );

		c = dtostrf(((float)db.gsmVolt/1000.0), 0, 3, cbuf );
		SEND( CF("gsmVolt"), "&%s=%s", cbuf );
		
		db.gpsLon=34.123456;
		db.gpsLat=32.123456;
		SEND( CF("gpsLon"), "&%s=32.123456", 32 );	//db.gpsLon );
		SEND( CF("gpsLat"), "&%s=32.123456", 45 );	//db.gpsLat );

		c = dtostrf(((float)db.bhvWeight/1000.0), 0, 3, cbuf );
		SEND( CF("bhvWeight"), "&%s=%s", cbuf );
		
#if 0															
		SEND( "apikey", "abcdefgh" );
		sprintf(_tempbuf, "%d", db->nodeId);
		SEND( "nodeId", _tempbuf );
		
		SEND( "mcuTemp", "55" );
		SEND( "batVolt", "3.995" );
		SEND( "bhvTemp", "36.56" );
		SEND( "bhvHumid", "54.32" );
		SEND( "rtcDateTime", "21-08-17_12:24" );
		SEND( "gsmSig", "12" );
		SEND( "gsmVolt", "4.02" );
		SEND( "gpsLon", "12.345678" );
		SEND( "gpsLat", "12.345678" );
		SEND( "bhvWeight", "123.456" );
#endif
		
		gsm_sendrecvcmdtimeout( CF( "\"\r\n" ), CF( "OK\r\n" ), 2 );
		

#if 0
		while(1) {
			if( gsm_available() )
				Serial.write( gsm_read() );
			if(Serial.available() )
				gsm_write( Serial.read() );
		}
#endif

		gsm_sendrecvcmdtimeout( CF ( "AT+HTTPACTION=0\r\n" ), CF( "+HTTPACTION:" ), 15 );

		CLEAR_TEMPBUF;
		READGSM( 5 );
		Serial.print("action: ");Serial.println( _tempbuf );
		
		c = strchr(_tempbuf, ',') + 1;
		
		c = strstr(c, CF( "200" ) );
		if(!c) {
			http_terminateRequest();
			return false;
		}
		
		c = strchr(c, ',');
		if(!c) {
			http_terminateRequest();
			return false;
		}
		
		dlen = atoi(++c);
		
		gsm_sendcmd( CF( "AT+HTTPREAD\r\n" ) );
		READGSM( 2 );
		
		while( gsm_available() )gsm_read();
	
	return (true);
}

bool gsm_moduleInfo()
{
	DEF_CLEAR_TEMPBUF;
	
  	gsm_flushInput();

  	gsmserial.print( CF( ("ATI\r\n") ) );
  	gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );
//  	Serial.print("GSM response: <"); Serial.print( _tempbuf ); Serial.println(">");
  	
  	_tempbuf[12] = '\0';
 // 	Serial.print(">>"); Serial.print(_tempbuf+6); Serial.println("<<");
  	
  	if(!strncmp( _tempbuf+6, "SIM800", 6 ))return true;
  	else return false;
}

bool gsm_getBattery(uint16_t &bat)
{
	char *c;
	DEF_CLEAR_TEMPBUF;

		
		if(!gsm_sendrecvcmdtimeout( CF( "AT+CBC\r\n" ), CF( "+CBC:" ), 2))
			return false;
		READGSM(2);
		
		c = strrchr( _tempbuf, ',' );
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
		gsm_sendcmd( CF( ("AT+CPIN=") ) );
		gsm_sendcmd( aspin );
		gsm_sendcmd( CF( ("\r\n") ) );
		if(!READGSM(2))return false;
		if(!strstr(_tempbuf, CF( ("OK\r\n") ) )) {
			Serial.println(CF( ( "Error setting cpin\n" ) ) );
			return false;
		}
		
	return true;
}


bool gsm_moduleReady()
{
	char *c;
	DEF_CLEAR_TEMPBUF;

	if( !gsm_sendrecvcmdtimeout( CF( ("AT\n\r") ) , CF( ("OK\r\n") ), 2) )return false;
	gsm_sendcmd( CF( ("AT+CREG?\n\r") )  );
	if( !READGSM( 5 ) )return false;
	c = strstr( _tempbuf, CF( ("+CREG: ") ) );
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

  	gsm_sendcmd( CF( ("AT+CREG?\n\r" ) ) );
  	READGSM( 2 );
  	
  	c = strstr(_tempbuf, CF( ("+CREG: ") ) );
  	if(!c)return false;
  	
  	c = strchr(c, ',');
  	areg = atoi( ++c );
#if 1
  		Serial.print(F("CREG network registration: ")); Serial.println( areg );
#endif
	/* result:
	 *	0		not registered, currently not searching for operator
	 *	1		registered, home network
	 *	2		not registered, current searching for operator
	 *  3		registration denied
	 *	4		unknown
	 *	5		registered, roaming
	 */
	
	return true;
}


bool gsm_moduleLowPower( bool alowpower )
{
	if( alowpower ) {
		if( gsm_sendrecvcmdtimeout( CF( ("AT+CFUNC=0\n\r") ) , CF( ("OK\r\n") ), 2) )return true;
		else return false;
	} else {
		if( gsm_sendrecvcmdtimeout(CF("AT+CFUNC=1\n\r"), CF("OK\r\n"), 2) )return true;
		else return false;
	}
}

bool gsm_getSignalQuality(uint8_t &asqual)
{
	char *c;
  DEF_CLEAR_TEMPBUF;

  	gsm_sendcmd( CF( ( "AT+CSQ\n\r" ) ) );
  	READGSM( 2 );
  	
  	c = strstr(_tempbuf, CF( ( "CSQ: " ) ) );
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

		gsm_sendcmd( CF("AT+CIPGSMLOC=1,1\n") );
		
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





bool http_getRequest(char *url, char *args, uint16_t &datalen)
{
	char *c;
	DEF_CLEAR_TEMPBUF;
	uint16_t result;

		gsm_sendcmd( CF( ("AT+HTTPPARA=\"URL\",\"") ) );
		gsm_sendcmd( url );
		gsm_sendcmd( CF( ("\",\"") ) );
		gsm_sendcmd( args );
		gsm_sendcmd( CF( ("\"\r\n") ) );
		
		READGSM( 2 );
		if(!strstr(_tempbuf, CF( ( "OK\r\n") ) ) ) {
			gsm_sendcmd( CF( ("AT+HTTPTERM\r\n") ) );
			return false;
		}
		
		if(!gsm_sendrecvcmdtimeout( CF( "AT+HTTPACTION=0\r\n" ), CF( "+HTTPACTION: 0,200" ), 35 )) {
			gsm_sendcmd( CF( "AT+HTTPTERM\r\n" ) );
			return false;
		}

#if 0
		gsm_sendcmd( CF( ("AT+HTTPACTION=0\r\n") ) );
		if( !READGSM( 15 ) ) {
			gsm_sendcmd( CF( ("AT+HTTPTERM\r\n") ) );
			return false;
		}
#endif

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

		READGSM( 2 );		 
		Serial.print(CF( "rd: " )); Serial.println( _tempbuf ); 
		c = strstr(_tempbuf, "+HTTPACTION");
		if(!c) {
			gsm_sendcmd( CF( ("AT+HTTPTERM\r\n") ) );
			return false;
		}
		       
		c = strchr(c, ',') + 1;

		/* only accept 200 (OK) request, all other are a fail so it must be
		 * treated as such */
		c = strstr(c, "200");
		if(!c) {
			gsm_sendcmd( CF( ("AT+HTTPTERM\r\n") ) );
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
			gsm_sendcmd( CF( ("AT+HTTPTERM\r\n") ) );
			return false;
		}
		
		datalen = atoi( ++c );
		
		gsmserial.print( CF( ("AT+HTTPREAD\r\n") ) );
		gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
				 
		/* clean receive buffer */
		while(gsmserial.available())gsmserial.read();
			
	return (result);
}
