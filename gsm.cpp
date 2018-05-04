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
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "rtc.h"
#include "gsm.h"
#include "mem.h"
//#include "data.h"


#define USE_NEOSW

#ifdef USE_NEOSW

#include <NeoSWSerial.h>
NeoSWSerial gsmserial(GSM_RX, GSM_TX);

#else

#include <SoftwareSerial.h>
SoftwareSerial gsmserial(GSM_RX, GSM_TX);

#endif	/* USE_NEOSW */


#define TEMP_BUF_LEN	64
#define	DEF_CLEAR_TEMPBUF	char _tempbuf[ TEMP_BUF_LEN ]; memset( _tempbuf, 0, TEMP_BUF_LEN )
#define CLEAR_TEMPBUF	memset( _tempbuf, 0, TEMP_BUF_LEN )

/* shortcut to read data in _tempbuf */
#define READGSM( lat )	gsm_readSerial( _tempbuf, TEMP_BUF_LEN, (lat) )

#define CF(str)	(char *)( str )
#define PF(str)	(PGM_P)F(str)


//const __FlashStringHelper *FOK = F("OK\r\n");

/* buf is buffer to store contents, buflen is length of buffer, timeout in seconds */
uint8_t gsm_readSerial(char *buf, uint8_t buflen, uint8_t timeout)
{
	char *c = buf;
	
	while((timeout) && (!gsmserial.available()) ) {
		delay(800);
		timeout -= 1;
	}
  
  if(!timeout)return false;
    
	/* while still room in buf and available characters from serial */
	while( (buflen) && (gsmserial.available()) ) {
		*buf++ = gsmserial.read();
		buflen--;
	}
    
	Serial.println( c );
        
	/* return with current buf */
	return true;
}


void gsm_init()
{
	gsmserial.begin( GSM_SERIAL_BAUDRATE );
}

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

bool gsm_sendrecvcmdp(const __FlashStringHelper *cmd, const __FlashStringHelper *expstr)
{
	DEF_CLEAR_TEMPBUF;

		while(gsm_available())gsm_read();
	  
		gsm_sendcmdp( cmd );
    
		if( READGSM( 2 ) ) {
			/* there was a string response read in 2sec */
			if(strstr_P(_tempbuf, (PGM_P)expstr)) {
				/* expected string found */
				return true;
			} else return false;
		} else return false;
}

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

bool gsm_sendrecvcmdtimeoutp(const __FlashStringHelper *cmd, const __FlashStringHelper *expstr, uint8_t timeout)
{
	DEF_CLEAR_TEMPBUF;
	uint32_t mils;
	char *tb;
  
		while(gsm_available())gsm_read();

		gsm_sendcmdp( cmd );

		mils = millis() + 1000UL * timeout;
		
		tb = _tempbuf;
		while( millis() < mils ) {
			while(gsm_available()) {
				if(strlen(_tempbuf) < TEMP_BUF_LEN-1) {
					*tb++ = gsm_read();
				} else {
					return false;
				}
			
				if(strstr_P(_tempbuf, (PGM_P)expstr)) {
					/* expected string found */
					return true;
				}
			}
		}
		
		/* timeout */
	return false;
}


#define SERIAL_DUMPCMD

void gsm_sendcmd(char *cmd)
{
	while(gsm_available())gsm_read();
	
	gsmserial.print( cmd );

#ifdef SERIAL_DUMPCMD
	Serial.print( cmd );
#endif
}

void gsm_sendcmdp(const __FlashStringHelper *cmd)
{
	while(gsm_available())gsm_read();
	
	gsmserial.print( cmd );

#ifdef SERIAL_DUMPCMD
	Serial.print( cmd );
#endif
}

		
void gsm_relayOutput( Stream &ast )
{
	while( gsmserial.available() )ast.write( gsmserial.read() );
}


bool gsm_activateBearerProfile()
{
//AT+SAPBR=3,1,"CONTYPE","GPRS"
//AT+SAPBR=3,1,"APN","myq"
//AT+SAPBR=3,1,"USER",""
//AT+SAPBR=3,1,"PWD",""

		/* reset interface */
		if(!gsm_sendrecvcmdtimeoutp( F("AT+CIPSHUT\r\n"), F("SHUT OK"), 2) )
			return false;
	
		if(!gsm_sendrecvcmdtimeoutp( F("AT+CIPMUX=0\r\n"), F("OK"), 2 ) )
			return false;

		/* set parameters */
    if(!gsm_sendrecvcmdtimeoutp( F( "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n") , F( "OK") , 2 ) )
    	return false;

		gsm_sendcmdp( F("AT+SAPBR=3,1,\"APN\",\"") );
		transmitEEPROMstr(E_APN, gsmserial);
		if(!gsm_sendrecvcmdtimeoutp( F( "\"\r\n") , F( "OK" ), 2 ) )
			return false;
		

		gsm_sendcmdp( F("AT+SAPBR=3,1,\"USER\",\"") );
		transmitEEPROMstr(E_USER, gsmserial);
		if(!gsm_sendrecvcmdtimeoutp( F( "\"\r\n" ), F( "OK" ), 2 ) )
			return false;
		

		gsm_sendcmdp( F("AT+SAPBR=3,1,\"PWD\",\"") );
		transmitEEPROMstr(E_PASS, gsmserial);
		if(!gsm_sendrecvcmdtimeoutp( F("\"\r\n"), F( "OK" ), 2 ) )
			return false;
			
    /* actual connection */
    if(!gsm_sendrecvcmdtimeoutp( F("AT+SAPBR=1,1\r\n"), F("OK"), 85) )
    return false;

    /* so we are connected! */
  return true;
}

uint8_t	gsm_getBearerStatus()
{
	char *c;
  DEF_CLEAR_TEMPBUF;
  

    gsm_sendcmdp( F("AT+SAPBR=2,1\r\n") );
    READGSM( 2 );

    c = strstr_P(_tempbuf, PF("+SAPBR: "));
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

		if(!gsm_sendrecvcmdtimeoutp( F("AT+SAPBR=0,1\r\n"), F( "OK" ), 65 ) )
			return false;

  return true;
}

bool gsm_initCIP()
{
	if(!gsm_sendrecvcmdtimeoutp( F("AT+CIPSHUT\r\n"), F("SHUT OK"), 2) )
		return false;
			
	if(!gsm_sendrecvcmdtimeoutp( F("AT+CIPMUX=0\r\n"), F("OK"), 2 ) )
		return false;
	
	gsm_sendcmdp( F( "AT+CSTT=\"") );
		
	transmitEEPROMstr(E_APN, gsmserial);
	gsm_sendcmdp( F("\",\"") );
		
	transmitEEPROMstr(E_USER, gsmserial);
	gsm_sendcmdp( F("\",\"") );

	transmitEEPROMstr(E_PASS, gsmserial);
		
	if(!gsm_sendrecvcmdtimeoutp( F( "\"\r\n" ), F( "OK" ), 2 ) )
		return false;

	if(!gsm_sendrecvcmdtimeoutp( F( "AT+CIPSTATUS\r\n" ), F( "IP START" ), 5 ) )
		return false;
		
	if(!gsm_sendrecvcmdtimeoutp( F( "AT+CIICR\r\n" ), F( "OK" ), 5 ) )
		return false;
		
	if(!gsm_sendrecvcmdtimeoutp( F( "AT+CIPSTATUS\r\n" ), F( "IP GPRSACT" ), 5 ) )
		return false;
		
	if(!gsm_sendrecvcmdtimeoutp( F( "AT+CIFSR\r\n" ), F( "." ), 5 ) )
		return false;

  return true;
}

bool gsm_doneCIP()
{
	if(!gsm_sendrecvcmdtimeoutp( F( "AT+CIPSHUT\r\n" ), F("SHUT OK"), 2 ))return false;
	else return true;
}



bool gsm_dnsLookup(uint8_t *ipaddr)
{
	DEF_CLEAR_TEMPBUF;
	char *c, *cc;

/*
 * AT+CIPSHUT					SHUT OK
 * AT+CIPMUX=0				OK
 * AT+CSTT="apn", "user", "pass"		OK
 * AT+CIPSTATUS											IP START
 * AT+CIICR													OK
 * AT+CIPSTATUS											IP GPRSACT
 * AT+CIFSR													0.0.0.0
 * AT+CDNSGIP="url.ext"							+CDNSGIP: 1,"url.ext","0.0.0.0"
 */

#if 0
		if(!gsm_sendrecvcmdtimeoutp( F("AT+CIPSHUT\r\n"), F("SHUT OK"), 2) )
			return false;
			
		if(!gsm_sendrecvcmdtimeoutp( F("AT+CIPMUX=0\r\n"), F("OK"), 2 ) )
			return false;
	
		c = _tempbuf;
		
		gsm_sendcmdp( F( "AT+CSTT=\"") );
		
		transmitEEPROMstr(E_APN, gsmserial);
		gsm_sendcmdp( F("\",\"") );
		
		transmitEEPROMstr(E_USER, gsmserial);
		gsm_sendcmdp( F("\",\"") );

		transmitEEPROMstr(E_PASS, gsmserial);
		
		if(!gsm_sendrecvcmdtimeoutp( F( "\"\r\n" ), F( "OK" ), 2 ) )
			return false;

		if(!gsm_sendrecvcmdtimeoutp( F( "AT+CIPSTATUS\r\n" ), F( "IP START" ), 5 ) )
			return false;
		
		if(!gsm_sendrecvcmdtimeoutp( F( "AT+CIICR\r\n" ), F( "OK" ), 5 ) )
			return false;
		
		if(!gsm_sendrecvcmdtimeoutp( F( "AT+CIPSTATUS\r\n" ), F( "IP GPRSACT" ), 5 ) )
			return false;
		
		if(!gsm_sendrecvcmdtimeoutp( F( "AT+CIFSR\r\n" ), F( "." ), 5 ) )
			return false;
#endif



		gsm_initCIP();
				
		gsm_sendcmdp( F( "AT+CDNSGIP=\"" ) );
		
		c = _tempbuf;
		CLEAR_TEMPBUF;
		c = getEEPROMstr(E_URL, c);
//		gsm_sendcmd( dns );
		gsm_sendcmd( c );

		if(!gsm_sendrecvcmdtimeoutp( F( "\"\r\n" ), F("+CDNSGIP:"), 20 ) )
			return false;
		
		CLEAR_TEMPBUF;
		READGSM( 5 );
		
		/* _tempbuf hold something like: < 1,"dns","5.55.2.216"\r\n> */				

//		Serial.print(">> ");Serial.println(_tempbuf);
		
		/* find last comma <,> */
		c = strrchr( _tempbuf, ',' );
		c++; // eat ,
		c++; // eat \"
		
//		/* copy string to ipstr */
//		strcpy( ipstr, c );

		/* ipstr hold: <5.55.2.216\"\r\n> */
		/* find last \" */
		cc = strchr( c, '\"');
		*cc = 0;		// null terminate string there

#define STRTOD(_v,_s)	_v=0;while(isdigit( *_s )) { _v=_v*10+(*_s - '0');_s++; }
	
		/* set ipaddr only if ipaddr pointer is non null */
		if(ipaddr) {
			STRTOD(ipaddr[0], c);c++;
			STRTOD(ipaddr[1], c);c++;
			STRTOD(ipaddr[2], c);c++;
			STRTOD(ipaddr[3], c);
		
//			Serial.println(ipaddr[0], DEC);
//			Serial.println(ipaddr[1], DEC);
//			Serial.println(ipaddr[2], DEC);
//			Serial.println(ipaddr[3], DEC);
		}

#if 0		
		gsm_sendrecvcmdtimeoutp( F( "AT+CIPSHUT\r\n" ), F("SHUT OK"), 2 );
#endif
	gsm_doneCIP();


	return true;
}
		

bool http_initiateGetRequest()
{
//	DEF_CLEAR_TEMPBUF;

		if(!gsm_sendrecvcmdtimeoutp( F( "AT+HTTPINIT\r\n" ), F( "OK" ),  2 ) ) {
			return false;
		}
		
		/* set HTTP CID */
		if(!gsm_sendrecvcmdtimeoutp( F( "AT+HTTPPARA=\"CID\",1\r\n" ), F( "OK" ), 2 ) ) {
			gsm_sendcmdp( F( "AT+HTTPTERM\r\n" ) );
			return false;
		}

		/* set HTTP USER AGENT */
		if(!gsm_sendrecvcmdtimeoutp( F( "AT+HTTPPARA=\"UA\",\"BEEHIVE SIM MODULE\"\r\n" ), F ("OK"), 2 ) ) {
			gsm_sendcmdp( F ("AT+HTTPTERM\r\n" ) );
			return false;
		}

	return (true);
}
		
void http_terminateRequest()
{
	gsm_sendrecvcmdtimeoutp( F( "AT+HTTPTERM\r\n" ) , F( "OK" ), 2);
}


#ifndef SERVER_URL
//#define SERVER_URL	CF( "5.55.150.188" )
#define SERVER_URL	CF( "erns.sytes.net" )

#endif

#ifndef SERVER_PORT
#define SERVER_PORT	CF( "8088" )
#endif

uint8_t serverip[4];


bool http_send_datablock(datablock_t &db)
{
	DEF_CLEAR_TEMPBUF;
	char	cbuf[20], *c;
	uint16_t dlen;

//		Serial.println( F("trying to send data block") );


		sprintf(cbuf, "%d.%d.%d.%d" , serverip[0],serverip[1],serverip[2],serverip[3]);

		Serial.print(F("Sending data to IP: ")); Serial.println( cbuf );


		gsm_sendcmdp( F( "AT+HTTPPARA=\"URL\",\"http://") );
		gsm_sendcmd( cbuf );

#if 0
//		gsm_sendcmd( SERVER_URL );
		CLEAR_TEMPBUF;
		if( getEEPROMstr(E_URL, _tempbuf) )
			gsm_sendcmd( _tempbuf );
		else gsm_sendcmdp( F("<unknown url>") );
#endif
	
		gsm_sendcmdp( F( ":" ) );
		

//		gsm_sendcmd( SERVER_PORT );
		CLEAR_TEMPBUF;
		gsm_sendcmd( utoa( getServerPort(), _tempbuf, 10 ) );


		gsm_sendcmdp( F( "/data.php?action=add" ) );
		

/* doSEND to 1, for old behavior, to 2 for new behavior */
//#define doSEND	1
#define doSEND	2

#if doSEND == 1

#define SEND(arg, fmt, value)	\
													sprintf(_tempbuf, fmt, arg, value); \
													gsm_sendcmd( _tempbuf )	;

//													Serial.println( _tempbuf ); 
#endif

#if doSEND == 2
#define SENDs(key, svalue) \
						gsm_sendcmdp(F("&")); \
						gsm_sendcmdp(key); \
						gsm_sendcmdp(F("=")); \
						gsm_sendcmd( svalue );

#define SENDsp(key, svalue) \
						gsm_sendcmdp(F("&")); \
						gsm_sendcmdp(key); \
						gsm_sendcmdp(F("=")); \
						gsm_sendcmdp( svalue );

#define SENDi(key, ivalue) \
						gsm_sendcmdp(F("&")); \
						gsm_sendcmdp(key); \
						gsm_sendcmdp(F("=")); \
						gsm_sendcmd( itoa( ivalue, _tempbuf, 10) );

#define SENDul(key, ivalue) \
						gsm_sendcmdp(F("&")); \
						gsm_sendcmdp(key); \
						gsm_sendcmdp(F("=")); \
						gsm_sendcmd( ultoa( ivalue, _tempbuf, 10) );

#define SENDf(key, fvalue, fsig) \
						gsm_sendcmdp(F("&")); \
						gsm_sendcmdp(key); \
						gsm_sendcmdp(F("=")); \
						gsm_sendcmd( dtostrf( fvalue, 1, fsig, _tempbuf ) );

#endif


#if doSEND == 1
		SEND( CF("apikey"), "&%s=%s", "abcdefgh" );
		SEND( CF("nodeId"), "&%s=%d", db.nodeId );
		SEND( CF("mcuTemp"), "&%s=%d", 100 );	//db.mcuTemp );  

		c = dtostrf(((float)db.batVolt/1000.0), 0, 3, cbuf );
		SEND( CF("batVolt"), "&%s=%s", cbuf );
		
		c = dtostrf(((float)db.bhvTemp/100.0), 0, 2, cbuf );
		SEND( CF("bhvTemp"), "&%s=%s", cbuf );


		c = dtostrf(((float)db.bhvHumid/100.0), 0, 2, cbuf );
		SEND( CF("bhvHumid"), "&%s=%s", cbuf );

		c = dtostrf(((float)db.bhvWeight/1000.0), 0, 3, cbuf );
		SEND( CF("bhvWeight"), "&%s=%s", cbuf );

#if 0
		sprintf(cbuf, "%02d-%02d-%02d_%02d:%02d",
			db.dt.dayOfMonth, 
			db.dt.month, 
			db.dt.year, 
			db.dt.hour, 
			db.dt.minute);
#endif

		sprintf(cbuf, "%02d-%02d-%02d_%02d:%02d",
			db.dayOfMonth, 
			db.month, 
			db.year, 
			db.hour, 
			db.minute);
//		Serial.println(cbuf);
		SEND( CF("rtcDateTime"), "&%s=%s", cbuf );
#endif



#if doSEND == 2
		SENDs(F("apikey"), getEEPROMstr( E_APIKEY, cbuf ) );
		SENDi(F("nodeId"), getNodeId());
//		SENDi(F("mcuTemp"), 100);

		switch(db.entryType) {
			case ENTRY_DATA:
				SENDsp(F("eType"), F("dat"));
				SENDf(F("batVolt"), db.batVolt/ 1000.0, 3);
				SENDf(F("bhvTemp"), db.bhvTemp / 100.0, 2);
				SENDf(F("bhvHumid"), db.bhvHumid / 100.0, 2);
				SENDf(F("bhvWeight"), db.bhvWeight / 1000.0, 3);
				break;
			case ENTRY_GSM:
				SENDsp(F("etype"), F("gsm"));
				SENDi(F("gsmSig"), db.gsmSig);
				SENDf(F("gsmVolt"), db.gsmVolt / 1000.0, 3);
				SENDul(F("gsmPDur"), db.gsmPowerDur);
				break;
			case ENTRY_GPS:
				SENDsp(F("etype"), F("gps"));
				SENDf(F("gsmLon"), db.gpsLon, 6);
				SENDf(F("gsmLat"), db.gpsLat, 6);
				break;
		
			default: break;
		}
		
		sprintf(cbuf, "%02d-%02d-%02d_%02d:%02d",
			db.dayOfMonth, 
			db.month, 
			db.year, 
			db.hour, 
			db.minute);
		SENDs(F("rtcDateTime"), cbuf );
#endif


		
#if 0
//		SEND( CF("rtcDateTime"), "&%s=%s", "22-9-17_10:51" );
		SEND( CF("gsmSig"), "&%s=%d", db.gsmSig );

		c = dtostrf(((float)db.gsmVolt/1000.0), 0, 3, cbuf );
		SEND( CF("gsmVolt"), "&%s=%s", cbuf );
		
		db.gpsLon=34.123456;
		db.gpsLat=32.123456;
		SEND( CF("gpsLon"), "&%s=%s", "32.32" );	//db.gpsLon );
		SEND( CF("gpsLat"), "&%s=%s", "45.45" );	//db.gpsLat );
#endif
		

		gsm_sendrecvcmdtimeoutp( F( "\"\r\n" ), F( "OK" ), 2 );
		

#if 0
		while(1) {
			if( gsm_available() )
				Serial.write( gsm_read() );
			if(Serial.available() )
				gsm_write( Serial.read() );
		}
#endif

		gsm_sendrecvcmdtimeoutp( F ( "AT+HTTPACTION=0\r\n" ), F( "+HTTPACTION:" ), 15 );

		CLEAR_TEMPBUF;
		READGSM( 5 );
		Serial.print( F("action: ") );Serial.println( _tempbuf );
		

		c = _tempbuf;

		c = strstr(c, "200" );
		if(!c) {
			Serial.println( F("response was not 200") );
			http_terminateRequest();
			return false;
		}
		
		c = strchr(c, ',');
		if(!c) {
			Serial.println( F("could not find received bytes length") );
			http_terminateRequest();
			return false;
		}
		
		dlen = atoi(++c);
		Serial.print( F("Received bytes: ") ); Serial.println( dlen );
		gsm_sendcmdp( F( "AT+HTTPREAD\r\n" ) );

//		delay(500);
//		READGSM( 2 );
		
#if 0
		/* flush input */
		while( gsm_available() )gsm_read();
#endif

#if 1
		while( gsm_available() ) {
			Serial.write( gsm_read() );
		}
#endif
			
	
	return (true);
}

bool gsm_moduleInfo()
{
	DEF_CLEAR_TEMPBUF;
	
		if(gsm_sendrecvcmdtimeoutp( F( "ATI\r\n" ), F( "OK" ), 2 ) )
			return true;
		else
			return false;
	

  	gsm_sendcmdp( F("ATI\r\n") );
  	READGSM( 2 );
//  	Serial.print("GSM response: <");
  	Serial.print( _tempbuf ); Serial.println( F(">") );
  	
//  	_tempbuf[12] = '\0';
 // 	Serial.print(">>"); Serial.print(_tempbuf+6); Serial.println("<<");
  	
 		if(strstr( _tempbuf, "OK" ) )return true;
 		else return false;
 		
  	if(!strncmp( _tempbuf+6, "SIM800", 6 ))return true;
  	else return false;
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


//bool gsm_sendPin(char *aspin)
bool gsm_sendPin()
{
//	DEF_CLEAR_TEMPBUF;

		gsm_sendcmdp( F("AT+CPIN=") );
		transmitEEPROMstr( E_SIMPIN, gsmserial );
//		gsm_sendcmd( aspin );

		if(!gsm_sendrecvcmdtimeoutp( F( "\r\n" ), F( "OK" ), 2 ) ) {
			Serial.println( F("Error setting SIM PIN") );
			return false;
		}
		
#if 0
		gsm_sendcmdp( F("\r\n") );
		if(!READGSM(2))return false;
		if(!strstr_P(_tempbuf, F( ("OK\r\n") ) )) {
			Serial.println(F( "Error setting cpin\n" ) );
			return false;
		}
#endif

	return true;
}

bool gsm_getICCID(char *aiccid)
{
	DEF_CLEAR_TEMPBUF;

		gsm_sendcmdp( F("AT+CCID\r\n") );
		
		if(!READGSM(5))return false;
		
		Serial.println(_tempbuf);
		
		for(int a=0;a<20;a++) {
			*aiccid++ = _tempbuf[a];
		}
	
		return (true);
}


bool gsm_moduleReady()
{
	uint8_t reg;
	DEF_CLEAR_TEMPBUF;

	if( !gsm_sendrecvcmdtimeoutp( F("AT\n\r") , F("OK"), 2) )return false;
	
	gsm_getRegistration( reg );
	if( (reg == 1) || (reg == 5))return true;
	else return false;
	 
#if 0
	gsm_sendcmdp( F("AT+CREG?\n\r") );
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
#endif

}


bool gsm_getRegistration(uint8_t &areg)
{
	char *c;
  DEF_CLEAR_TEMPBUF;

  	gsm_sendcmdp( F("AT+CREG?\n\r" ) );
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
		if( gsm_sendrecvcmdtimeoutp( F("AT+CFUNC=0\n\r") , F("OK"), 2) )return true;
		else return false;
	} else {
		if( gsm_sendrecvcmdtimeoutp(F("AT+CFUNC=1\n\r"), F("OK"), 2) )return true;
		else return false;
	}
}

bool gsm_getBattery(uint16_t &bat)
{
	char *c;
	DEF_CLEAR_TEMPBUF;

		
		if(!gsm_sendrecvcmdtimeoutp( F( "AT+CBC\r\n" ), F( "+CBC:" ), 2))
			return false;
		
		READGSM(2);
		
		c = strrchr( _tempbuf, ',' );
		if(!c)return false;
		
		bat = atoi( ++c );

	return true;
}

bool gsm_getSignalQuality(uint8_t &asqual)
{
	char *c;
  DEF_CLEAR_TEMPBUF;

		if(!gsm_sendrecvcmdtimeoutp( F( "AT+CSQ\r\n" ), F( "+CSQ: " ), 2))
			return false;

  	READGSM( 2 );
		c = _tempbuf;
  	if(!c)return false;

  	asqual = atoi( c );
		return true;
		
	return true;
}

bool gsm_getDateTimeLonLat(uint8_t &hour, uint8_t &min, uint8_t &sec, uint8_t &day, uint8_t &month,
uint8_t &year, float &lon, float &lat)
{
	char *c;
	int16_t res;
	DEF_CLEAR_TEMPBUF;

		gsm_sendcmdp( F("AT+CIPGSMLOC=1,1\n") );
		
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

bool gsm_getDateTime(uint8_t &hour, uint8_t &min, uint8_t &sec, uint8_t &day, uint8_t &month,
uint8_t &year)
{
	char *c;
	int16_t res;
	DEF_CLEAR_TEMPBUF;

		gsm_sendcmdp( F("AT+CIPGSMLOC=2,1\n") );
		
		c=strstr( _tempbuf, "GSMLOC: " );
		if(!c)return false;
	
		/* reponse is:
		 * +CIPGSMLOC:<locationcode>[,<date>,<time>]
		 * where:
		 *  <locationcode> is 0 if command succesful, or i.e. 404 NOT FOUND
		 *  <date>	YYYY/MM/DD
		 *  <time>	hh/mm/ss
		 */
		 
		//<locationcode>
		res = atoi( ++c );
		c++;
		STRTOD(res, c);
		/* res holds value of <locationcode>, and c points to comma <,> */
		if(res != 0)return false;
		/* res could be:
		 * 0		Success
		 * 404	Not found
		 * 408	Request time-out
		 * 601	Network error
		 * 602	No memory
		 * 603	DNS error
		 * 604	Stack busy
		 * 65535	other error
		 */
		
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

		gsm_sendcmdp( F("AT+HTTPPARA=\"URL\",\"") );
		gsm_sendcmd( url );
		gsm_sendcmdp( F("\",\"") );
		gsm_sendcmd( args );
		gsm_sendcmdp( F("\"\r\n") );
		
		READGSM( 2 );
		if(!strstr(_tempbuf, CF( ( "OK\r\n") ) ) ) {
			gsm_sendcmdp( F("AT+HTTPTERM\r\n") );
			return false;
		}
		
		if(!gsm_sendrecvcmdtimeoutp( F( "AT+HTTPACTION=0\r\n" ), F( "+HTTPACTION: 0,200" ), 35 )) {
			gsm_sendcmdp( F("AT+HTTPTERM\r\n") );
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

		READGSM( 2 );		 
		Serial.print(F( "rd: " )); Serial.println( _tempbuf ); 
		c = strstr(_tempbuf, "+HTTPACTION");
		if(!c) {
			gsm_sendcmdp( F( "AT+HTTPTERM\r\n") );
			return false;
		}
		       
		c = strchr(c, ',') + 1;

		/* only accept 200 (OK) request, all other are a fail so it must be
		 * treated as such */
		c = strstr(c, "200");
		if(!c) {
			gsm_sendcmdp( F("AT+HTTPTERM\r\n") );
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
			gsm_sendcmdp( F("AT+HTTPTERM\r\n") );
			return false;
		}
		
		datalen = atoi( ++c );
		
		gsmserial.print( CF( ("AT+HTTPREAD\r\n") ) );
		gsm_readSerial(_tempbuf, TEMP_BUF_LEN, 2 );		/* for timeout see SIM800 command manual */
				 
		/* clean receive buffer */
		while(gsmserial.available())gsmserial.read();
			
	return (result);
}
