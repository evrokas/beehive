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
#include "pstr.h"
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


/* buf is buffer to store contents, buflen is length of buffer, timeout in seconds */
uint8_t gsm_readSerial(char *buf, uint8_t buflen, uint8_t timeout)
{
	timeout *= 2;
	while((timeout) && (!gsmserial.available()) ) {
		delay(500);
		timeout--;;
	}
  
  if(!timeout)return false;
    

  while(timeout) {
  	/* while still room in buf and available characters from serial */
  	while( (buflen) && (gsmserial.available()) ) {
			*buf = gsmserial.read();


//			Serial.print( *buf );


			buf++;
			buflen--;
		}
		
		timeout--;
		delay(500);
	}
    
//	Serial.println( c );
        
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

//		while(gsm_available())gsm_read();
	  
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

//		while(gsm_available()) Serial.print( gsm_read() );
	  
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
  
//		while(gsm_available())gsm_read();

		gsm_sendcmd( cmd );

		mils = millis() + 1000UL * timeout;
		
		tb = _tempbuf;
		while( millis() < mils ) {
			while(gsm_available()) {
				if(strlen(_tempbuf) < TEMP_BUF_LEN-1) {
					*tb++ = gsm_read();

//					Serial.println( _tempbuf );
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
  
//		while(gsm_available())Serial.print( gsm_read() );

		gsm_sendcmdp( cmd );

		mils = millis() + 1000UL * timeout;
		
		tb = _tempbuf;
		while( millis() < mils ) {
			while(gsm_available()) {
				if(strlen(_tempbuf) < TEMP_BUF_LEN-1) {
					*tb++ = gsm_read();
//					Serial.println( _tempbuf );
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




//#define SERIAL_DUMPCMD

void gsm_sendcmd(char *cmd)
{
	while(gsm_available())gsm_read();
	
	gsmserial.print( cmd );

#ifdef SERIAL_DUMPCMD
	Serial.print( cmd );
#endif
}

void gsm_sendchar(char ch)
{
	gsmserial.print( ch );
	
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


#if defined(HTTP_API_POST)
/*
 * the gsm_postcmd* version of functions is used in conjuction
 * with POST HTTP method end Transfer-Encoding: chunked
 * Actually, they buffer output and print output in chunks of
 * predetermined size */

#define CHUNK_BUFFER_SIZE		16
uint8_t _chunk_pos; 
char chunk_buffer[ CHUNK_BUFFER_SIZE+1 ];



void gsm_poststart()
{
	_chunk_current = 0;
	memset( chunk_buffer, 0, CHUNK_BUFFER_SIZE+1 );
}


void gsm_postcmd(char *cmd)
{
  DEF_CLEAR_TEMPBUF;
  uint8_t i=0;
  
	while( i < strlen( cmd ) ) {
		
		if( _chunk_pos < CHUNK_BUFFER_SIZE ) {
			/* there is still room in buffer */
			chunk_buffer[ _chunk_pos ] = cmd[i];
			_chunk_pos++;
			i++;
			continue;
		} else {
			uint8_t n;
			
			/* buffer is full, so print result */
			gsm_sendcmd( itoa( ivalue, _tempbuf, 10) );			/* print number of bytes */
			gsm_sendcmdp( RCF( pCRLF ) );
			gsm_sendcmd( chunk_buffer );	/* always chunk_buffer[ CHUNK_BUFFER ] is \0 */
			gsm_sendcmdp( RCF( pCRLF ) );
			gsm_poststart();
		}
	}
}

void gsm_postcmdp(const __FlashStringHelper *cmd)
{
  DEF_CLEAR_TEMPBUF;
  uint8_t i=0;
  
	while( i < strlen( cmd ) ) {
		
		if( _chunk_pos < CHUNK_BUFFER_SIZE ) {
			/* there is still room in buffer */
			chunk_buffer[ _chunk_pos ] = cmd[i];
			_chunk_pos++;
			i++;
			continue;
		} else {
			uint8_t n;
			
			/* buffer is full, so print result */
			gsm_sendcmd( itoa( strlen( chunk_buffer ), _tempbuf, 10) );			/* print number of bytes */
			gsm_sendcmdp( RCF( pCRLF ) );
			gsm_sendcmd( chunk_buffer );	/* always chunk_buffer[ CHUNK_BUFFER ] is \0 */
			gsm_sendcmdp( RCF( pCRLF ) );
			gsm_poststart();
		}
	}
}

void gsm_postdone()
{
	DEF_CLEAR_TEMPBUF;
	uint8_t i=0;

		if(_chunk_pos) {
			/* there are still data in buffer, spit them... */
			chunk_buffer[ _chunk_pos ] = 0;

			gsm_sendcmd( itoa( strlen( chunk_buffer ), _tempbuf, 10) );			/* print number of bytes */
			gsm_sendcmdp( RCF( pCRLF ) );
			gsm_sendcmd( chunk_buffer );
			gsm_sendcmdp( RCF( pCRLF );
		}
		
		gsm_sendcmdp( F("0\r\n\r\n");		/* this marks the end of the chunks */
		gsm_poststart();
}
		
#endif	/* HTTP_API_POST */


		
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
		if(!gsm_sendrecvcmdtimeoutp( RCF(pATCIPSHUTrn), RCF(pSHUTOK), 2) )
			return false;

//		if(!gsm_sendrecvcmdtimeoutp( F("AT+CIPSHUT\r\n"), F("SHUT OK"), 2) )
//			return false;
	
		if(!gsm_sendrecvcmdtimeoutp( RCF(pATCIPMUX0rn), RCF(pOK), 2 ) )
			return false;

		/* set parameters */
    if(!gsm_sendrecvcmdtimeoutp( RCF(pATSAPBR31CONTYPE) , RCF(pOK) , 2 ) )
    	return false;

		gsm_sendcmdp( RCF(pATSAPBR31APN) );
		transmitEEPROMstr(E_APN, gsmserial);
		if(!gsm_sendrecvcmdtimeoutp( RCF(pQrn) , RCF(pOK), 2 ) )
			return false;
		

		gsm_sendcmdp( RCF(pATSAPBR31USER) );
		transmitEEPROMstr(E_USER, gsmserial);
		if(!gsm_sendrecvcmdtimeoutp( RCF( pQrn ), RCF( pOK ), 2 ) )
			return false;
		

		gsm_sendcmdp( RCF( pATSAPBR31PWD ) );
		transmitEEPROMstr(E_PASS, gsmserial);
		if(!gsm_sendrecvcmdtimeoutp( RCF( pQrn ), RCF( pOK ), 2 ) )
			return false;
			
    /* actual connection */
    if(!gsm_sendrecvcmdtimeoutp( RCF( pATSAPBR11rn ), RCF( pOK ), 85) )
    return false;

    /* so we are connected! */
  return true;
}

uint8_t	gsm_getBearerStatus()
{
	char *c;
  DEF_CLEAR_TEMPBUF;
  

    gsm_sendcmdp( RCF( pATSAPBR21rn ) );
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

		if(!gsm_sendrecvcmdtimeoutp( RCF( pATSAPBR01rn ), RCF( pOK ), 65 ) )
			return false;

  return true;
}

bool gsm_initCIP()
{
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPSHUTrn ), RCF( pSHUTOK ), 2) )
		return false;
			
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPMUX0rn ), RCF( pOK ), 2 ) )
		return false;
	
	gsm_sendcmdp( RCF( pATCSTTQ ) );
		
	transmitEEPROMstr(E_APN, gsmserial);
	gsm_sendcmdp( RCF( pQcommaQ ) );
		
	transmitEEPROMstr(E_USER, gsmserial);
	gsm_sendcmdp( RCF( pQcommaQ ) );

	transmitEEPROMstr(E_PASS, gsmserial);
		
	if(!gsm_sendrecvcmdtimeoutp( RCF( pQrn ), RCF( pOK ), 2 ) )
		return false;

	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPSTATUSrn ), RCF( pIPSTART ), 5 ) )
		return false;
		
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIICRrn ), RCF( pOK ), 5 ) )
		return false;
		
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPSTATUSrn ), RCF( pIPGPRSACT ), 5 ) )
		return false;
		
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIFSRrn ), RCF( pDOT ), 5 ) )
		return false;

  return true;
}

bool gsm_doneCIP()
{
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPSHUTrn ), RCF( pSHUTOK ), 2 ))return false;
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
//		gsm_initCIP();
				
		gsm_sendcmdp( RCF( pATCDNSGIPQ ) );
		
		c = _tempbuf;
		CLEAR_TEMPBUF;
		c = getEEPROMstr(E_URL, c);
//		gsm_sendcmd( dns );
		gsm_sendcmd( c );

		if(!gsm_sendrecvcmdtimeoutp( RCF( pQrn ), RCF( pCDNSGIP ), 20 ) )
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

/*
			Serial.println(ipaddr[0], DEC);
			Serial.println(ipaddr[1], DEC);
			Serial.println(ipaddr[2], DEC);
			Serial.println(ipaddr[3], DEC);
*/
		}

#if 0		
		gsm_sendrecvcmdtimeoutp( RCF( pATCIPSHUTrn ), RCF( pSHUTOK ), 2 );
#endif
//		gsm_doneCIP();

	return true;
}
		

bool http_initiateGetRequest()
{
//	DEF_CLEAR_TEMPBUF;

		if(!gsm_sendrecvcmdtimeoutp( RCF( pATHTTPINITrn ), RCF( pOK ),  2 ) ) {
			return false;
		}
		
		/* set HTTP CID */
		if(!gsm_sendrecvcmdtimeoutp( RCF( pATHTTPPARACIDrn ), RCF( pOK ), 2 ) ) {
			gsm_sendcmdp( RCF( pATHTTPTERMrn ) );
			return false;
		}

		/* set HTTP USER AGENT */
		if(!gsm_sendrecvcmdtimeoutp( RCF( pATHTTPPARAUArn ), RCF ( pOK ), 2 ) ) {
			gsm_sendcmdp( RCF ( pATHTTPTERMrn ) );
			return false;
		}

	return (true);
}
		
void http_terminateRequest()
{
	gsm_sendrecvcmdtimeoutp( RCF( pATHTTPTERMrn ) , RCF( pOK ), 2);
}


//#ifndef SERVER_URL
//#define SERVER_URL	CF( "5.55.150.188" )
//#define SERVER_URL	CF( "erns.sytes.net" )

//#endif

//#ifndef SERVER_PORT
//#define SERVER_PORT	CF( "8088" )
//#endif

uint8_t serverip[4];


void transmitServerIP()
{
	char cbuf[20];

		sprintf_P(cbuf, (PGM_P)F("%d.%d.%d.%d") , serverip[0],serverip[1],serverip[2],serverip[3]);
		gsm_sendcmd( cbuf );
}

#if defined( HTTP_API_POST )

bool http_post_db_preample(uint16_t nid)
{
	DEF_CLEAR_TEMPBUF;
	
	//{"action":"add","nodeId":100,"data":);
	gsm_postcmdp( RCF( pCURLOPEN ) );
	
		POSTSENDsp("action", "add");
		POSTSENDcomma;
		POSTSENDi("nodeId", nid);
		gsm_sendcmdp(",\"data\":[");
}

bool http_post_db_postample()
{
	gsm_postcmdp( F("]}"));
}


bool http_post_db_data(datablock_t &db)
{
	DEF_CLEAR_TEMPBUF;
	
		gsm_sendcmdp( RCF( pCURLOPEN );
	
	
		switch(db.entryType) {
			case ENTRY_DATA:
				POSTSENDsp(RCF( pentryType ), RCF( pDAT ));
				POSTSENDcomma;
				POSTSENDf(RCF( pbatVolt ), db.batVolt/ 1000.0, 3);
				POSTSENDcomma;
				POSTSENDf(RCF( pbhvTemp ), db.bhvTemp / 100.0, 2);
				POSTSENDcomma;
				POSTSENDf(RCF( pbhvHumid ), db.bhvHumid / 100.0, 2);
				POSTSENDcomma;
				POSTSENDf(RCF( pbhvWeight ), db.bhvWeight / 1000.0, 3);
				break;
			case ENTRY_GSM:
				POSTSENDsp(RCF( pentryType ), RCF( pGSM ));
				POSTSENDcomma;
				POSTSENDi(RCF( pgsmSig ), db.gsmSig);
				POSTSENDcomma;
				POSTSENDf(RCF( pgsmVolt ), db.gsmVolt / 1000.0, 3);
				POSTSENDcomma;
				POSTSENDul(RCF( pgsmPDur ), db.gsmPowerDur);
				break;
			case ENTRY_GPS:
				POSTSENDsp(RCF( pentryType ), RCF( PGPS ));
				POSTSENDcomma;
				POSTSENDf(RCF( pgpsLon ), db.gpsLon, 6);
				POSTSENDcomma;
				POSTSENDf(RCF( pgpsLat ), db.gpsLat, 6);
				break;
		
			default: break;
		}
		
		POSTSENDcomma;
		sprintf_P(cbuf, (PGM_P)F("%02d-%02d-%02d_%02d:%02d"),
			db.dayOfMonth, 
			db.month, 
			db.year, 
			db.hour, 
			db.minute);

		POSTSENDs(RCF( prtcDateTime ), cbuf );
		
		gsm_sendcmdp( RCF( pCURLCLOSE );

  return (true);
}



bool http_send_post()
{
	datablock_t db;
	uint16_t ii;
	uint8_t iii;
			
	gsm_sendcmdp( F("POST /data.php HTTP/1.1\n") );
	gsm_sendcmdp( F("Host: 10.0.0.1\n" ) );
	gsm_sendcmdp( F("User-Agent: beewatch-firmware/0.1\n");
	gsm_sendcmdp( F("Content-Type: application/json\n") );
	gsm_sendcmdp( F("Transfer-Encoding: chunked\n") );
	
	gsm_sendcmdp( RCF( pCRLF ) );
		
//	gsm_sendcmdp( F("Content-Length: ") );

	/* start emitting chunked data */
	gsm_poststart();
	
	http_post_db_preample( getNodeId() );

	while( mem_popDatablock( &db ) ) {
		http_post_db_data( db );
		
		SENDPOSTcomma;
	}
	/* so all data blocks have been send */
	
	/* send the final GSM block */
	db.entryType = ENTRY_GSM;

	db.entryType = ENTRY_GSM;
	if( gsm_getBattery( ii ) ) {
//		Serial.print("Battery level: " ); Serial.println( ii );
		db.gsmVolt = ii;
	}

	if( gsm_getSignalQuality( iii ) ) {
//		Serial.print("Signal quality: " ); Serial.println( iii );
		db.gsmSig = iii;
	}
						
	db.gsmPowerDur = millis() - mil1;

	http_post_db_data( db );
	
	http_post_db_postample();
	
	/* send any remaining data from the buffer */
	gsm_postdone();

  return (true);
}

#endif	/* HTTP_API_POST */


#if defined( HTTP_API_GET )

bool http_send_datablock_get(datablock_t &db)
{
	DEF_CLEAR_TEMPBUF;
	char	cbuf[20], *c;
	uint16_t dlen;

//		Serial.println( F("trying to send data block") );


//		sprintf(cbuf, "%d.%d.%d.%d" , serverip[0],serverip[1],serverip[2],serverip[3]);

//		Serial.print(F("Sending data to IP: ")); Serial.println( cbuf );

		gsm_sendcmdp( RCF( pATHTTPPARAURLQ ) );
//		gsm_sendcmd( cbuf );
		transmitServerIP();

		gsm_sendcmdp( RCF( pDDOT ) );

		CLEAR_TEMPBUF;
		gsm_sendcmd( utoa( getServerPort(), _tempbuf, 10 ) );

		gsm_sendcmdp( F( "/data.php?action=add" ) );
		

		memset(cbuf, 0, sizeof( cbuf ));
//		SENDs(RCF( pAPIKEY ), getEEPROMstr( E_APIKEY, cbuf ) );
		GETSENDi(RCF( pnodeId ), getNodeId());

		switch(db.entryType) {
			case ENTRY_DATA:
				GETSENDsp(RCF( pentryType ), RCF( pDAT ));
				GETSENDf(RCF( pbatVolt ), db.batVolt/ 1000.0, 3);
				GETSENDf(RCF( pbhvTemp ), db.bhvTemp / 100.0, 2);
				GETSENDf(RCF( pbhvHumid ), db.bhvHumid / 100.0, 2);
				GETSENDf(RCF( pbhvWeight ), db.bhvWeight / 1000.0, 3);
				break;
			case ENTRY_GSM:
				GETSENDsp(RCF( pentryType ), RCF( pGSM ));
				GETSENDi(RCF( pgsmSig ), db.gsmSig);
				GETSENDf(RCF( pgsmVolt ), db.gsmVolt / 1000.0, 3);
				GETSENDul(RCF( pgsmPDur ), db.gsmPowerDur);
				break;
			case ENTRY_GPS:
				GETSENDsp(RCF( pentryType ), RCF( PGPS ));
				GETSENDf(RCF( pgpsLon ), db.gpsLon, 6);
				GETSENDf(RCF( pgpsLat ), db.gpsLat, 6);
				break;
		
			default: break;
		}
		

		sprintf_P(cbuf, (PGM_P)F("%02d-%02d-%02d_%02d:%02d"),
			db.dayOfMonth, 
			db.month, 
			db.year, 
			db.hour, 
			db.minute);

		GETSENDs(RCF( prtcDateTime ), cbuf );

		gsm_sendrecvcmdtimeoutp( RCF( pQrn ), RCF( pOK ), 2 );
		
		gsm_sendrecvcmdtimeoutp( RCF ( pATHTTPACTION0rn ), RCF( pHTTPACTION ), 15 );

		CLEAR_TEMPBUF;
		READGSM( 5 );
		Serial.print( F("action: ") );Serial.println( _tempbuf );
		
		c = _tempbuf;

		c = strstr_P(c, (PGM_P)F("200") );
		if(!c) {
			Serial.println( F("response was not 200") );
			Serial.println( _tempbuf );
			http_terminateRequest();
			return false;
		}
		
		c = strchr(c, ',');
		if(!c) {
			Serial.println( RCF( pNoReceivedLength ) );
			http_terminateRequest();
			return false;
		}
		
		dlen = atoi(++c);
		Serial.print( F("Received bytes: ") ); Serial.println( dlen );
		gsm_sendcmdp( RCF( pATHTTPREADrn ) );

//		delay(500);
//		READGSM( 2 );
		
#if 0
		/* flush input */
		while( gsm_available() )gsm_read();
#endif

#if 1
		delay( 200 );
		while( gsm_available() ) {
			Serial.write( gsm_read() );
		}
#endif
			
	
	return (true);
}

#endif	/* HTTP_API_GET */

void gsm_interactiveMode()
{
  char c;
		Serial.println(F("Interactive mode (enter '~' to continue)"));
		while(1) {
			if( gsm_available() )
				Serial.write( gsm_read() );

			if(Serial.available() ) {
				c = Serial.read();
				if(c == '~')break;
				gsm_write( c );
			}
		}
}


#if defined( HTTP_API_GET )
	
bool http_send_getconf_request()
{
	DEF_CLEAR_TEMPBUF;
	char	*c;
	uint16_t dlen;

//		sprintf(cbuf, "%d.%d.%d.%d" , serverip[0],serverip[1],serverip[2],serverip[3]);

//		Serial.print(F("Sending data to IP: ")); Serial.println( cbuf );

		gsm_sendcmdp( RCF( pATHTTPPARAURLQ ) );

		transmitServerIP();
		
//		gsm_sendcmd( cbuf );

		gsm_sendcmdp( RCF( pDDOT ) );

		CLEAR_TEMPBUF;
		gsm_sendcmd( utoa( getServerPort(), _tempbuf, 10 ) );

		gsm_sendcmdp( F( "/data.php?action=getconf" ) );

		GETSENDi(RCF( pnodeId ), getNodeId());
		
		gsm_sendrecvcmdtimeoutp( RCF( pQrn ), RCF( pOK ), 2 );
		
		gsm_sendrecvcmdtimeoutp( RCF( pATHTTPACTION0rn ), RCF( pHTTPACTION ), 15 );

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
			Serial.println( RCF( pNoReceivedLength ) );
			http_terminateRequest();
			return false;
		}
		
		Serial.print(F("response text: "));Serial.println( _tempbuf );
		
		dlen = atoi(++c);
		Serial.print( F("Received bytes: ") ); Serial.println( dlen );
//		delay(200);

		

		//gsm_interactiveMode();

		if(!gsm_sendrecvcmdtimeoutp( RCF( pATHTTPREADrn ), RCF( pHTTPREAD ), 2 ) )
		{
			return true;
		}
		
		CLEAR_TEMPBUF;
		READGSM( 5 );
		Serial.print(F(">>")); Serial.println( _tempbuf );

		do {
			uint16_t nid;
			uint16_t lc, nc;
			char *uc = _tempbuf;

			
			while( *uc++ != '\n');					/* eat characters read after +HTTPREAD */
			while( !isdigit( *uc ) )uc++;		/* eat all characters until 1 digit */
			
			STRTOD( nid, uc ); uc++;
			STRTOD( lc, uc ); uc++;
			STRTOD( nc, uc );
			
			Serial.print(F("NodeId: ")); Serial.println( nid );
			Serial.print(F("Log cycle: ")); Serial.println( lc );
			Serial.print(F("Net cycle: ")); Serial.println( nc );
		
			if( lc != getLogCycle())
				maxMinLogCycle = lc;	//setLogCycle( lc );
			
			if( nc != getNetCycle() )
				maxMinNetCycle = nc;	//setNetCycle( nc );
				
		} while( 0 );
		
		
#if 0
		while( gsm_available() ) {
			Serial.write( gsm_read() );
		}
#endif


	return (true);
}

#endif	/* HTTP_API_GET */

bool gsm_moduleInfo()
{
	DEF_CLEAR_TEMPBUF;
	
		if(gsm_sendrecvcmdtimeoutp( RCF( pATIrn ), RCF( pOK ), 2 ) )
			return true;
		else
			return false;
	

  	gsm_sendcmdp( RCF( pATIrn ) );
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

		gsm_sendcmdp( RCF( pATCPINeq ) );
		transmitEEPROMstr( E_SIMPIN, gsmserial );
//		gsm_sendcmd( aspin );

		if(!gsm_sendrecvcmdtimeoutp( RCF( pRN ), RCF(  pOK ), 2 ) ) {
			Serial.println( RCF( pErrorCPIN ) );
			return false;
		}
		gsm_sendcmdp( RCF( pATrn ) );
		
#if 0
		gsm_sendcmdp( RCF( pRN ) );
		if(!READGSM(2))return false;
		if(!strstr_P(_tempbuf, F( ("OK\r\n") ) )) {
			Serial.println(RCF( pErrorCPIN ) );
			return false;
		}
#endif

	return true;
}

bool gsm_getICCID(char *aiccid)
{
	DEF_CLEAR_TEMPBUF;

		gsm_sendcmdp( RCF( pATCCIDrn ) );
		
		if(!READGSM(2))return false;
		
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

	if( !gsm_sendrecvcmdtimeoutp( RCF( pATrn ) , RCF( pOK ), 2) )return false;
	
	gsm_getRegistration( reg );
	if( (reg == 1) || (reg == 5))return true;
	else return false;
}


bool gsm_getRegistration(uint8_t &areg)
{
	char *c;
  DEF_CLEAR_TEMPBUF;

  	gsm_sendcmdp( RCF( pATCREGqrn ) );
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
		if( gsm_sendrecvcmdtimeoutp( RCF( pATCFUNC0rn ) , RCF( pOK ), 2) )return true;
		else return false;
	} else {
		if( gsm_sendrecvcmdtimeoutp( RCF( pATCFUNC0rn ), RCF( pOK ), 2) )return true;
		else return false;
	}
}

bool gsm_getBattery(uint16_t &bat)
{
	char *c;
	DEF_CLEAR_TEMPBUF;

		
		if(!gsm_sendrecvcmdtimeoutp( RCF( pATCBCrn ), RCF( pCBC ), 2))
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

		if(!gsm_sendrecvcmdtimeoutp( RCF( pATCSQrn ), RCF( pCSQ ), 2))
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

		gsm_sendcmdp( RCF( pATCIPGSMLOC11rn ) );
		
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

		gsm_sendcmdp( RCF( pATCIPGSMLOC21rn ) );
		
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

#if 0

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
#endif
