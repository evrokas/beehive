/*
 * gsm.h - GSM/GPRS module header
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-17
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#ifndef __GSM_H__
#define __GSM_H__

#define GSM_TX	10
#define GSM_RX	9

#ifndef GSM_SERIAL_BAUD
#define GSM_SERIAL_BAUDRATE	9600
#endif

//#include "data.h"
#include "mem.h"

#define PREAMPLE_SIZE	71		/* until ...Length:_ */

/* lengths with string literals, using numeric literals will even reduce size */
#if defined( API_STRING_KEYS )

#define SIZE_DAT			119
#define SIZE_GSM			96
#define SIZE_GPS			91

#elif defined( API_NUMERIC_KEYS )

#define SIZE_DAT			75
#define SIZE_GSM			62
#define SIZE_GPS			65

#endif



/* macros for GET method  */
#define GETSENDs(key, svalue) \
						gsm_sendcmdp(RCF( pAmp )); \
						gsm_sendcmdp(key); \
						gsm_sendcmdp(RCF( pEq )); \
						gsm_sendcmd( svalue );

#define GETSENDsp(key, svalue) \
						gsm_sendcmdp(RCF( pAmp )); \
						gsm_sendcmdp(key); \
						gsm_sendcmdp(RCF( pEq )); \
						gsm_sendcmdp( svalue );

#define GETSENDi(key, ivalue) \
						gsm_sendcmdp(RCF( pAmp )); \
						gsm_sendcmdp(key); \
						gsm_sendcmdp(RCF( pEq )); \
						gsm_sendcmd( itoa( ivalue, _tempbuf, 10) );

#define GETSENDul(key, ivalue) \
						gsm_sendcmdp(RCF( pAmp )); \
						gsm_sendcmdp(key); \
						gsm_sendcmdp(RCF( pEq )); \
						gsm_sendcmd( ultoa( ivalue, _tempbuf, 10) );

#define GETSENDf(key, fvalue, fsig) \
						gsm_sendcmdp(RCF( pAmp )); \
						gsm_sendcmdp(key); \
						gsm_sendcmdp(RCF( pEq )); \
						gsm_sendcmd( dtostrf( fvalue, 1, fsig, _tempbuf ) );


/* macros for POST method  */
#define POSTSENDs(key, svalue) \
						gsm_postcmdp(RCF( pQ )); \
						gsm_postcmdp(key); \
						gsm_postcmdp(RCF( pQddQ )); \
						gsm_postcmd( svalue );	\
						gsm_postcmd(RCF( pQ ));

#define POSTSENDsp(key, svalue) \
						gsm_postcmdp(RCF( pQ )); \
						gsm_postcmdp( key ); \
						gsm_postcmdp(RCF( pQddQ )); \
						gsm_postcmdp( svalue ); \
						gsm_postcmdp(RCF( pQ ) );

#define POSTSENDi(key, ivalue) \
						gsm_postcmdp(RCF( pQ )); \
						gsm_postcmdp(key); \
						gsm_postcmdp(RCF( pQddQ )); \
						gsm_postcmd( itoa( ivalue, _tempbuf, 10) ); \
						gsm_postcmdp(RCF( pQ ));

#define POSTSENDul(key, ivalue) \
						gsm_postcmdp(RCF( pQ )); \
						gsm_postcmdp(key); \
						gsm_postcmdp(RCF( pQddQ )); \
						gsm_postcmd( ultoa( ivalue, _tempbuf, 10) ); \
						gsm_postcmdp(RCF( pQ ));

#define POSTSENDf(key, fvalue, fsig) \
						gsm_postcmdp(RCF( pQ )); \
						gsm_postcmdp(key); \
						gsm_postcmdp(RCF( pQddQ )); \
						gsm_postcmd( dtostrf( fvalue, 1, fsig, _tempbuf ) ); \
						gsm_postcmdp(RCF( pQ ));

#define POSTSENDcomma	gsm_postcmdp( RCF( pCOMMA ) );


extern "C" {

    void gsm_init(void);

    bool gsm_sendrecvcmd(char *cmd, char *expstr);
    bool gsm_sendrecvcmdtimeout(char *cmd, char *expstr, uint8_t timeout);
    void gsm_sendcmd(char *cmd);

    void gsm_sendcmdp(const __FlashStringHelper *cmd);
    bool gsm_sendrecvcmdp(const __FlashStringHelper *cmd, const __FlashStringHelper *expstr);
    bool gsm_sendrecvcmdtimeoutp(const __FlashStringHelper *cmd, const __FlashStringHelper *expstr, uint8_t timeout);

    void gsm_relayOutput( Stream &ast );
    
    bool gsm_activateBearerProfile();
    uint8_t	gsm_getBearerStatus();
    bool gsm_deactivateBearerProfile();
    

    bool http_initiateGetRequest();
    void http_terminateRequest();
    
//    uint16_t http_getRequest(char *url, char *args);
#if defined( HTTP_API_GET )
    bool http_send_datablock_get(datablock_t &db);
    bool http_send_getconf_request();
#endif

#if defined( HTTP_API_POST )
		bool http_send_post_preample();
#endif

    bool gsm_moduleInfo();

		bool gsm_getBattery(uint16_t &bat);
	    
    bool gsm_available();
    char gsm_read();
    void gsm_write(char c);
    
    void gsm_flushInput();

//    bool gsm_sendPin(char *apin);    
    bool gsm_sendPin();    
    bool gsm_getRegistration(uint8_t &areg);
		bool gsm_moduleReady();
		bool gsm_getICCID(char *aiccid);


		bool gsm_moduleLowPower( bool alowpower );
		bool gsm_getSignalQuality(uint8_t &asqual);
	
		bool gsm_getDateTime(uint8_t &hour, uint8_t &min, uint8_t &sec, uint8_t &day, uint8_t &month, uint16_t &year);
		bool gsm_dnsLookup(uint8_t *ipaddr);
		bool gsm_initCIP();
		bool gsm_doneCIP();

		extern uint8_t serverip[4];

		unsigned long getGSMCharCount();
		void resetGSMCharCount();
		void incGSMCharCount(unsigned int aincr = 1);
	
		
};

#endif	/* __GSM_H__ */
