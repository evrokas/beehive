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

#define GSM_TX	13
#define GSM_RX	14

#define GSM_DEFAULT_BAUDRATE	19200


extern "C" {

    void gsm_init(void);

    int gsm_sendrecvcmd(char *cmd, char *expstr);
};



#endif	/* __GSM_H__ */
