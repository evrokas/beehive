/*
 * error.h - error handling functionality
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

#ifndef __ERROR_H__
#define __ERROR_H__

#define ERROR_BUFFER_LENGTH	512

extern char __error_buffer[ ERROR_BUFFER_LENGTH ];
extern uint16_t __error_head;
extern uint16_t __error_tail;


/* define error codes */
#define erCANNOTPUSHDBINEEPROM			1		/* S: "could not push data block in eeprom" */
#define erCANNOTSENDDAT							2		/* S: "cannot send DAT block" */
#define erCANNOTSENDGSM							3		/* S: "cannot send GSM block" */
#define erCANNOTINITGETREQ					4		/* S: "cannot initiate GET request" */
#define erCANNOTACTIVATEBEARER			5		/* S: "cannot activate bearer profile" */
#define erCANNOTINITPOST						6		/* S: "could not initiate POST request" */




#ifdef __cplusplus
extern "C" {
#endif

void initErrorHandling();
void doneErrorHandling();

bool logError(uint8_t erType);

#ifdef __cplusplus
}
#endif



#endif	/* __ERROR_H__ */
