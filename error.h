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

#include <stdint.h>
#include "bms.h"

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
#define erPOSTSENDFAILED						7		/* S: "sending records via POST failed" */


typedef struct {
	uint16_t 		sc_time;			/* sleep cycle, error occured */
	uint8_t			er_code;			/* error code */
	union {
		uint8_t		dat_ui8[5];				/* uint8_t arguments */
		uint16_t	dat_ui16[2];			/* uint16_t arguments */
		uint32_t	dat_ui32[1];			/* uint32_t arguments */
		float			dat_float;				/* float argument */
	};
} errorRecord;



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
