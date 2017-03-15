/*
 * gsm.h -GSM/GPRS module header
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


extern "C" {

    void gsm_init(void);

    bool gsm_sendrecvcmd(char *cmd, char *expstr);
    
    bool gsm_activateBearerProfile(char *apn, char *user, char *pass);
    
    uint8_t	gsm_getBearerStatus();
    
    bool gsm_deactivateBearerProfile();
    
    uint16_t http_getRequest(char *url, char *args);

    bool gsm_moduleInfo();
	uint16_t gsm_getBattery();
	    
    bool gsm_available();
    char gsm_read();
    void gsm_write(char c);
    
    void gsm_flushinput();
    
};



#endif	/* __GSM_H__ */
