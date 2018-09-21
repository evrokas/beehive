/*
 * gsm.cpp - GSM/GPRS Module functions file
 *
 * BeeWatch Project
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-18
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#include <avr/pgmspace.h>

#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>

#include "bms.h"
#include "pstr.h"
#include "utils.h"
#include "rtc.h"
#include "gsm.h"
#include "mem.h"

#ifdef USE_NEOSW

#include <NeoSWSerial.h>
NeoSWSerial gsmserial(GSM_RX, GSM_TX);

#else

#include <SoftwareSerial.h>
SoftwareSerial gsmserial(GSM_RX, GSM_TX);

#endif	/* USE_NEOSW */


/* this holds the IP address of the server, which is updated in each NET cycle,
 * if DNS resolving has failed, then it uses previous succesful resolve */
uint8_t serverip[4];


/* buf is buffer to store contents, buflen is length of buffer, timeout in seconds
 * sender must take to zero fill the buffer (if needed) */
uint8_t gsm_readSerial(char *buf, uint8_t buflen, uint8_t timeout)
{
	timeout *= 2; /* double the timeout value, since we are going to wait in steps
								 * of 1/2 of a second or 500msec */
								 
	while((timeout) && (!gsmserial.available()) ) {
		delay(500);
		timeout--;;
	}
  
	/* check if timed out */
  if(!timeout)return false;
    

  /* if not, then something is on the gsmserial buffer, read it on */
  while(timeout) {
  	/* while still room in buf and available characters from serial */
  	while( (buflen) && (gsmserial.available()) ) {
			*buf++ = gsmserial.read();
			buflen--;
		}
		
		timeout--;
		delay(500);
	}
    
	return true;
}


#define SERIAL_DUMPCMD

/* send 'cmd' to gsmserial, cmd is stored in RAM */
void gsm_sendcmd(char *cmd)
{
	while(gsm_available())gsm_read();
	
	gsmserial.print( cmd );

#ifdef SERIAL_DUMPCMD
	Serial.print( cmd );
#endif
}

/* send 'ch' to gsmserial, ch is a character in RAM */
void gsm_sendchar(char ch)
{
	while(gsm_available())gsm_read();

	gsmserial.print( ch );
	
#ifdef SERIAL_DUMPCMD
	Serial.print( ch );
#endif
}

/* send 'cmd' to gsmserial, cmd is stored in FLASH memory */
void gsm_sendcmdp(const __FlashStringHelper *cmd)
{
	while(gsm_available())gsm_read();
	
	gsmserial.print( cmd );

#ifdef SERIAL_DUMPCMD
	Serial.print( cmd );
#endif
}


/* send 'cmd' and expect 'expstr' as response, return true is respnose
 * is received, false otherwise */
bool gsm_sendrecvcmd(char *cmd, char *expstr)
{
	DEF_CLEAR_TEMPBUF;

		gsm_sendcmd( cmd );
    
		if( READGSM( 2 ) ) {
			/* there was a string response read in 2sec */
			if(strstr(_tempbuf, expstr)) {
				/* expected string found */
				return true;
			} else return false;
		} else return false;
}

/* send 'cmd' string in FLASH and expext 'expstr' string in FLASH,
 * return true is response was received, false otherwise */
bool gsm_sendrecvcmdp(const __FlashStringHelper *cmd, const __FlashStringHelper *expstr)
{
	DEF_CLEAR_TEMPBUF;

#if 0
		D("send: "); Dln( cmd );
		D("recv: "); Dln( expstr );
#endif

		gsm_sendcmdp( cmd );
    
		if( READGSM( 2 ) ) {
			/* there was a string response read in 2sec */
			if(strstr_P(_tempbuf, (PGM_P)expstr)) {
				/* expected string found */
				return true;
			} else return false;
		} else return false;
}


/* send 'cmd' and expect 'expstr' but timeout after 'timeout' seconds */
bool gsm_sendrecvcmdtimeout(char *cmd, char *expstr, uint8_t timeout)
{
	DEF_CLEAR_TEMPBUF;
	uint32_t mils;
	char *tb;
  
#if 0
		D("send: "); Dln( cmd );
		D("recv: "); Dln( expstr );
#endif

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

/* same as above, but strings are placed in FLASH memory */
bool gsm_sendrecvcmdtimeoutp(const __FlashStringHelper *cmd, const __FlashStringHelper *expstr, uint8_t timeout)
{
	DEF_CLEAR_TEMPBUF;
	uint32_t mils;
	char *tb;
  
//		while(gsm_available())Serial.print( gsm_read() );

#if 0
		D("send: "); Dln( cmd );
		D("recv: "); Dln( expstr );
#endif

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


void gsm_init()
{
	gsmserial.begin( GSM_SERIAL_BAUDRATE );
}

		
void gsm_relayOutput( Stream &ast )
{
	while( gsmserial.available() )ast.write( gsmserial.read() );
}


bool gsm_initCIP()
{

	/* AT+CIPSHUT 		SHUT OK */
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPSHUTrn ), RCF( pSHUTOK ), 2) )
		return false;
	
	/* AT+CIPMUX=0 		OK */
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPMUX0rn ), RCF( pOK ), 2 ) )
		return false;
	
	/* AT+CSTT="apn","user", "pass" 		OK */
	gsm_sendcmdp( RCF( pATCSTTQ ) );
		
	transmitEEPROMstr(E_APN, gsmserial);
	gsm_sendcmdp( RCF( pQcommaQ ) );
		
	transmitEEPROMstr(E_USER, gsmserial);
	gsm_sendcmdp( RCF( pQcommaQ ) );

	transmitEEPROMstr(E_PASS, gsmserial);
		
	if(!gsm_sendrecvcmdtimeoutp( RCF( pQrn ), RCF( pOK ), 2 ) )
		return false;

	/* AT+CIPSTATUS		IP START */
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPSTATUSrn ), RCF( pIPSTART ), 5 ) )
		return false;
		
	/* AT+CIICR				OK */
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIICRrn ), RCF( pOK ), 5 ) )
		return false;
		
	/* AT+CIPSTATUS		IP GPRSACT */
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPSTATUSrn ), RCF( pIPGPRSACT ), 5 ) )
		return false;
	
	/* AT+CIFSR				#.#.#.# */
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIFSRrn ), RCF( pDOT ), 5 ) )
		return false;

  return true;
}

bool gsm_doneCIP()
{
	if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPSHUTrn ), RCF( pSHUTOK ), 2 ))return false;
	else return true;
}

/* perform DNS lookup for HTTP type of requests,
 * always remember to bring up the wireless prior to calling
 * this function, with a call to gsm_initCIP() */
bool gsm_dnsLookup(uint8_t *ipaddr)
{
	DEF_CLEAR_TEMPBUF;
	char *c, *cc;

 /* perform DNS resolv
 * AT+CDNSGIP="url.ext"							+CDNSGIP: 1,"url.ext","0.0.0.0"
 */

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

	return true;
}


/* format and send IP address to gsmserial */
void transmitServerIP()
{
	char cbuf[20];

		sprintf_P(cbuf, (PGM_P)F("%d.%d.%d.%d") , serverip[0],serverip[1],serverip[2],serverip[3]);
		gsm_sendcmd( cbuf );
}

/* send an ATI command to gsmserial and return true if OK received, otherwise false */
bool gsm_moduleInfo()
{
		if(gsm_sendrecvcmdtimeoutp( RCF( pATIrn ), RCF( pOK ), 2 ) )
			return true;
		else
			return false;
}

#ifdef GSM_FUNCS_AS_MACRO

/* since we just call gsmserial.availabke() making this a macro,
 * might save some code */
/* this is defined in gsm.h */
//#define gsm_available()		(gsmserial.available())

#else

/* return true is character is available in gsmserial */
bool gsm_available()
{
	return (gsmserial.available());
}

/* return character from gsmserial */
char gsm_read()
{
	return (gsmserial.read());
}


void gsm_write(char c)
{
	gsmserial.write(c);
}

#endif	/* GSM_FUNCS_AS_MACRO */

void gsm_flushInput()
{
	while( gsmserial.available() )
		gsmserial.read();
}


/* send pin to gsmserial, does not wait for response,
 * NOTE: if pin was set incorrectly, then it might lock
 * the SIM card */
bool gsm_sendPin()
{
		gsm_sendcmdp( RCF( pATCPINeq ) );
		transmitEEPROMstr( E_SIMPIN, gsmserial );

		if(!gsm_sendrecvcmdtimeoutp( RCF( pRN ), RCF(  pOK ), 2 ) ) {
			Serial.println( RCF( pErrorCPIN ) );
			return false;
		}
		gsm_sendcmdp( RCF( pATrn ) );
		
	return true;
}

/* reads the CCID of the SIM card, return true if read ok,
 * otherwise false */
bool gsm_getICCID(char *aiccid)
{
	DEF_CLEAR_TEMPBUF;

		gsm_sendcmdp( RCF( pATCCIDrn ) );
		delay(200);
		
		READGSM(2);
		if(!strlen(_tempbuf))return false;

		Dln(_tempbuf);
		
		for(int a=0;a<20;a++) {
			*aiccid++ = _tempbuf[a];
		}
	
		return (true);
}

/* return true is module is ready and registered in the network */
bool gsm_moduleReady()
{
	uint8_t reg;
	
	gsm_getRegistration( reg );
	if( (reg == 1) || (reg == 5))return true;
	else return false;
}

/* return true if could get registration status, otherwise false,
 * registration status is returned in areg according to the comment below */
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

#if 0
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

/* set/unset the module to low power mode */
bool gsm_moduleLowPower( bool alowpower )
{
	if( alowpower ) {
		if( gsm_sendrecvcmdtimeoutp( RCF( pATCFUNC0rn ) , RCF( pOK ), 2) )return true;
		else return false;
	} else {
		if( gsm_sendrecvcmdtimeoutp( RCF( pATCFUNC1rn ), RCF( pOK ), 2) )return true;
		else return false;
	}
}

/* get battery level */
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
		
		D("Battery voltage: "); Dln( bat );
	return true;
}

/* get signal quality */
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

  	D("Signal: "); Dln( asqual );
  	
		return true;
		
	return true;
}

/* get Date, Time, Longitude and Latitude from the network,
 * returns true, is succesull, otherwise false */
bool gsm_getDateTimeLonLat(uint8_t &hour, uint8_t &min, uint8_t &sec, uint8_t &day, uint8_t &month,
uint8_t &year, float &lon, float &lat)
{
	char *c;
	int16_t res;
	DEF_CLEAR_TEMPBUF;

		if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPGSMLOC11rn ), RCF( pCIPGSMLOC ),  10))
			return false;

		READGSM(5);

		c = _tempbuf;
		//c=strstr_P( _tempbuf, PF("+CIPGSMLOC: "));
		//if(!c)return false;
	
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


		if(!gsm_sendrecvcmdtimeoutp( RCF( pATCIPGSMLOC21rn ), RCF( pCIPGSMLOC ),  10))
			return false;

		READGSM(5);

		c = _tempbuf;
	
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

/* interactive mode to the module, used for debugging purposes */
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
