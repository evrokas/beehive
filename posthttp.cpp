/*
 * posthttp.cpp - HTTP POST functionality
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


#if defined(HTTP_API_POST)
/*
 * the gsm_postcmd* version of functions is used in conjuction
 * with POST HTTP method and Transfer-Encoding: chunked
 * Actually, they buffer output and print output in chunks of
 * predetermined size of CHUNK_BUFFER_SIZE */

#define CHUNK_BUFFER_SIZE		32
uint8_t _chunk_pos; 
char chunk_buffer[ CHUNK_BUFFER_SIZE+1 ];

void gsm_poststart()
{
	_chunk_pos = 0;
	memset( chunk_buffer, 0, CHUNK_BUFFER_SIZE+1 );
}

void gsm_sendchunk()
{
	char tmp[5];
	
//		D("<>");D(chunk_buffer);D("\t");Dln(strlen( chunk_buffer ) );
	
		sprintf(tmp, "%02x", strlen( chunk_buffer ) );
		gsm_sendcmd( tmp );
		
		/* CRLF */
//		gsm_sendcmdp( RCF( pCRLF ) );
		gsm_sendcmdp( RCF( pLF ) );
			
		/* chunk data follow */
		gsm_sendcmd( chunk_buffer );	/* chunk_buffer[ CHUNK_BUFFER ] is always \0 */
//		gsm_sendcmdp( RCF( pCRLF ) );
		gsm_sendcmdp( RCF( pLF ) );
}

//#define DEBUG_POSTCMD
	
void gsm_postcmd(char *cmd)
{
  DEF_CLEAR_TEMPBUF;
  uint8_t i=0;
  
#ifdef DEBUG_POTSCMD
  D("*s "); D(cmd); D(" ("); D( strlen(cmd) ); Dln(")");
#endif
	
	while( i < strlen( cmd ) ) {
		
		if( _chunk_pos < CHUNK_BUFFER_SIZE ) {
			/* there is still room in buffer */
#ifdef DEBUG_POSTCMD
			D(">> _chunk_pos: "); D(_chunk_pos); D("\ti: ");D(i); D("\tchar: "); Dln(cmd[i]);
#endif


/*
			chunk_buffer[ _chunk_pos ] = cmd[i];
			_chunk_pos++;
			i++;
*/
			chunk_buffer[ _chunk_pos++ ] = cmd[ i++ ];
			
//			continue;
		} else {
			/* buffer is full, so print result */
//			Dln(">> sending chunk");
			gsm_sendchunk();

			/* reset chunk buffer */
			gsm_poststart();
		}
	}
}

/* same as above, but works with strings in flash memory */
void gsm_postcmdp(const __FlashStringHelper *cmd)
{
  DEF_CLEAR_TEMPBUF;
  uint8_t i=0;
  PGM_P cp = reinterpret_cast<PGM_P>(cmd);
  
#ifdef DEBUG_POSTCMD
  D("*p ");D( cmd );D(" (");D(strlen_P( (PGM_P)cmd ) ); Dln(")");
#endif

	while( i < strlen_P( (PGM_P)cmd ) ) {
		
		if( _chunk_pos < CHUNK_BUFFER_SIZE ) {
			/* there is still room in buffer */
#ifdef DEBUG_POSTCMD
			D(">> _chunk_pos: "); D(_chunk_pos); D("\ti: ");D(i); D("\tchar: ");
			//Dln(((PGM_P)cmd)[i]);
			//Dln( (char)pgm_read_byte( cp ) );
#endif


			chunk_buffer[ _chunk_pos ] = pgm_read_byte( cp );	//((PGM_P)cmd)[i];
			_chunk_pos++;
			i++;
			cp++;
			continue;
		} else {
			/* buffer is full, so print result */

			gsm_sendchunk();
			
			/* reset chunk buffer */
			gsm_poststart();
		}
	}
}

void gsm_postdone()
{
	DEF_CLEAR_TEMPBUF;

		if(_chunk_pos) {
			/* there are still data in buffer, spit them out ... */
			chunk_buffer[ _chunk_pos ] = 0;

			gsm_sendchunk();
		}
		
//		gsm_sendcmdp( F("0\r\n\r\n\r\n") );		/* this marks the end of the chunks */
		gsm_sendcmdp( F("0\n\n\n\n") );		/* this marks the end of the chunks */
		gsm_poststart();
}
		
bool gsm_initiateCIPRequest()
{
	DEF_CLEAR_TEMPBUF;
	
	/*
	 * AT+CIPSTART="TCP", "IP", "PORT"
	 * AT+CIPSEND
	 */
	 
	gsm_sendcmdp( RCF( pATCIPSTART ) );				/* AT+CIPSTART="TCP", " */
//	transmitServerIP();								/* *.*.*.* */
	transmitEEPROMstr( E_URL, gsmserial, true );
	gsm_sendcmdp( RCF( pQcommaQ ) );	/* "," */
	
	gsm_sendcmd( utoa( getServerPort(), _tempbuf, 10 ) );	/* port */
	
	if(!gsm_sendrecvcmdtimeoutp( RCF( pQrn ), RCF( pOK ), 2 ))
		return false;
	
/*
	gsm_sendcmdp( RCF( pATCIPSEND ) );	
	
	delay(2);	/* allow '>' character to come in */
*/
	
	/* so connection if ready to transmit useable data */
  return (true);
}
	
bool http_post_db_preample(uint16_t nid)
{
	DEF_CLEAR_TEMPBUF;
	
//	gsm_poststart();
	//{"action":"add","nodeId":100,"data":);
	gsm_postcmdp( RCF( pCURLOPEN ) );
	
		POSTSENDsp( F("action") , F("add") );
		POSTSENDcomma;
		POSTSENDi( RCF( pnodeId ), getNodeId() );
		gsm_postcmdp( F(",\"data\":[") );

  return (true);
}

bool http_post_db_postample()
{
	gsm_postcmdp( F("]}"));
  return (true);
}

bool http_post_db_data(datablock_t &db)
{
	DEF_CLEAR_TEMPBUF;
	
		gsm_postcmdp( RCF( pCURLOPEN ) );
	
	
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
		
		sprintf_P(_tempbuf, (PGM_P)F("%02d-%02d-%02d_%02d:%02d"),
			db.dayOfMonth, db.month, db.year, 
			db.hour, db.minute);

		POSTSENDs( RCF( prtcDateTime ), _tempbuf );
		
		gsm_postcmdp( RCF( pCURLCLOSE ) );

  return (true);
}

bool http_send_post(unsigned long amsecs)
{
	datablock_t db;
	uint16_t ii;
	uint8_t iii;
	
	gsm_getBattery( ii );
	gsm_getSignalQuality( iii );


	gsm_sendcmdp( RCF( pATCIPSEND ) );	
	
	delay(2);	/* allow '>' character to come in */

			
//	gsm_sendcmdp( F("POST /data.php HTTP/1.1\n") );
	gsm_sendcmdp( F("POST /post.php HTTP/1.1\n") );
	gsm_sendcmdp( F("Host: 10.0.0.1\n" ) );
	gsm_sendcmdp( F("User-Agent: beewatch-firmware/0.1\n") );
	gsm_sendcmdp( F("Content-Type: application/json\n") );
	gsm_sendcmdp( F("Transfer-Encoding: chunked\n") );
	
//	gsm_sendcmdp( RCF( pCRLF ) );
	gsm_sendcmdp( RCF( pLF ) );
		
	/* start emitting chunked data */
	gsm_poststart();
	
	http_post_db_preample( getNodeId() );

	while( mem_popDatablock( &db ) ) {
		http_post_db_data( db );
		
		POSTSENDcomma;
	}
	/* so all data blocks have been send */
	
	/* send the final GSM block */
	db.entryType = ENTRY_GSM;
	//if( gsm_getBattery( ii ) ) {
//		Serial.print("Battery level: " ); Serial.println( ii );
		db.gsmVolt = ii;
	//}

	//if( gsm_getSignalQuality( iii ) ) {
//		Serial.print("Signal quality: " ); Serial.println( iii );
		db.gsmSig = iii;
	//}
						
	db.gsmPowerDur = millis() - amsecs;

	http_post_db_data( db );
	
	http_post_db_postample();
	
	/* send any remaining data from the buffer */
	gsm_postdone();

//	gsm_interactiveMode();

//	gsm_sendcmdp( RCF( pCtrlZ ) );

  return (true);
}

#endif	/* HTTP_API_POST */
