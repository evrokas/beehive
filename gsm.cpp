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
#include <SoftwareSerial.h>
#include "gsm.h"



SoftwareSerial gsmserial(GSM_RX, GSM_TX);
char tempbuf[16], *tch;

void gsm_init()
{
    gsmserial.begin( GSM_DEFAULT_BAUDRATE );
}

int gsm_sendrecvcmd(char *cmd, char *expstr)
{
    gsmserial.println( cmd );
    
    tch = tempbuf;
    while( gsmserial.available() ) {
        *tch = gsmserial.read();
        if(*tch == '\n')break;
        if(tch < (tempbuf + sizeof( tempbuf )))tch++;
    }
    
    *tch = '\0';
    
    return 1;
}
