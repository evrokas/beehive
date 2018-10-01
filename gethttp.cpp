/*
 * gethttp.cpp - HTTP GET functionality
 *
 * BeeWatch Project
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-18
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 */


#include "bms.h"
#include "gsm.h"
#include "pstr.h"
#include "utils.h"
#include "mem.h"


#if defined( HTTP_API_GET )

bool gsm_activateBearerProfile()
{
//AT+SAPBR=3,1,"CONTYPE","GPRS"
//AT+SAPBR=3,1,"APN","myq"
//AT+SAPBR=3,1,"USER",""
//AT+SAPBR=3,1,"PWD",""

		/* reset interface */
		if(!gsm_sendrecvcmdtimeoutp( RCF(pATCIPSHUTrn), RCF(pSHUTOK), 2) )
			return false;

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

	
bool http_send_getconf_request()
{
	DEF_CLEAR_TEMPBUF;
	char	*c;
	uint16_t dlen;

//		sprintf(cbuf, "%d.%d.%d.%d" , serverip[0],serverip[1],serverip[2],serverip[3]);

//		Serial.print(F("Sending data to IP: ")); Serial.println( cbuf );

		gsm_sendcmdp( RCF( pATHTTPPARAURLQ ) );
		transmitServerIP();
		gsm_sendcmdp( RCF( pDDOT ) );

		CLEAR_TEMPBUF;
		gsm_sendcmd( utoa( getServerPort(), _tempbuf, 10 ) );

		gsm_sendcmdp( F( "/data.php?action=getconf" ) );

		GETSENDi(RCF( pnodeId ), getNodeId());
		
		gsm_sendrecvcmdtimeoutp( RCF( pQrn ), RCF( pOK ), 2 );
		
		gsm_sendrecvcmdtimeoutp( RCF( pATHTTPACTION0rn ), RCF( pHTTPACTION ), 15 );

		CLEAR_TEMPBUF;
		READGSM( 5 );
		D( F("action: ") );Dln( _tempbuf );
		
		c = _tempbuf;

		c = strstr(c, "200" );
		if(!c) {
			Dln( F("response was not 200") );
			http_terminateRequest();
			return false;
		}
		
		c = strchr(c, ',');
		if(!c) {
			Dln( RCF( pNoReceivedLength ) );
			http_terminateRequest();
			return false;
		}
		
		D(F("response text: "));Dln( _tempbuf );
		
		dlen = atoi(++c);
		D( F("Received bytes: ") ); Dln( dlen );
		

		//gsm_interactiveMode();

		if(!gsm_sendrecvcmdtimeoutp( RCF( pATHTTPREADrn ), RCF( pHTTPREAD ), 2 ) )
		{
			return true;
		}
		
		CLEAR_TEMPBUF;
		READGSM( 5 );
		Serial.print(F(">>")); Serial.println( _tempbuf );

		/* response from server:
		 * nid,log_period,net_period
		 */

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

			if( nid == getNodeId() ) {
				if( lc != getLogCycle())
					maxMinLogCycle = lc;	//setLogCycle( lc );
			
				if( nc != getNetCycle() )
					maxMinNetCycle = nc;	//setNetCycle( nc );
			}
							
		} while( 0 );
		
	return (true);
}

#endif	/* HTTP_API_GET */
