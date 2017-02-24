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

#define GSM_DEFAULT_BAUDRATE	19200


extern "C" {

    void gsm_init(void);

    bool gsm_sendrecvcmd(char *cmd, char *expstr);
    
    bool gsm_activateBearerProfile(char *apn, char *user, char *pass);
    
    uint8_t	gsm_getBearerStatus();
    
    bool gsm_deactivateBearerProfile();
    
    uint16_t http_getRequest(char *url, char *args);

};



#endif	/* __GSM_H__ */
