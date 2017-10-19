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

#include "data.h"

extern "C" {

    void gsm_init(void);

    bool gsm_sendrecvcmd(char *cmd, char *expstr);
    bool gsm_sendrecvcmdtimeout(char *cmd, char *expstr, uint8_t timeout);
    void gsm_sendcmd(char *cmd);

    void gsm_relayOutput( Stream &ast );
    
    bool gsm_activateBearerProfile(char *apn, char *user, char *pass);
    
    uint8_t	gsm_getBearerStatus();
    
    bool gsm_deactivateBearerProfile();
    

    bool http_initiateGetRequest();
    void http_terminateRequest();
    
    uint16_t http_getRequest(char *url, char *args);
    bool http_send_datablock(datablock_t &db);
    
    bool gsm_moduleInfo();

		bool gsm_getBattery(uint16_t &bat);
	    
    bool gsm_available();
    char gsm_read();
    void gsm_write(char c);
    
    void gsm_flushInput();

    bool gsm_sendPin(char *apin);    
    bool gsm_getRegistration(uint8_t &areg);
		bool gsm_moduleReady();

		bool gsm_moduleLowPower( bool alowpower );
		bool gsm_getSignalQuality(uint8_t &asqual);
	
		bool gsm_getDateTime(uint8_t &hour, uint8_t &min, uint8_t &sec, uint8_t &day, uint8_t &month, uint16_t &year);
		bool gsm_dnsLookup(char *apn, char *user, char *pass, char *dns, char *ipstr, uint8_t *ipaddr);

		extern uint8_t serverip[4];
		
};



#endif	/* __GSM_H__ */
