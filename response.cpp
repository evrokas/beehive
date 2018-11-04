/*
 * response.cpp - parse server response functions
 *
 * Beehive Monitoring System - BMS
 * (c) Evangelos Rokas, 2015-18
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

//#include <Arduino.h>
#include <ctype.h>
#include <string.h>

#include "bms.h"
#include "response.h"


#define STRTOD(_v,_s)	_v=0;while(isdigit( *_s )) { _v=_v*10+(*_s - '0');_s++; }


#ifdef LINUX_NATIVE_APP
#include <stdio.h>
#define PRINTF(str)		printf("%s", str);
#define PRINTFn(str)	printf("%s\n", str);
#define PUTCHAR(c)		putchar(c);
#else
#define	PRINTF(str)
#define PRINTFn(str)
#define PUTCHAR(c)
#endif


/* use a bit struct to reduce RAM usage */
struct {
	uint8_t respAction:4;			/* action codes encoded in 4 bits ==> 16 different actions */
	uint8_t respState:2;			/* states are, 0=initial, 1=command, 2=payload */
} _rv;
char resp[ RESPONSE_STR_SIZE ];

void initResponseParser()
{
	memset(resp, 0, sizeof(resp));
	_rv.respAction = 0;
	_rv.respState = 0;
}

/* commands come as a stream like:
 * 1,21,3,43,15 (last number should always be 15 (= 0xf == raSTOP))
 */

bool parseResponse(char ch)
{
	if( ch != ',') {
		/* do not pass to next state, but add to the respStr instead */
//		PUTCHAR( ch );
		strncat(resp, &ch, 1);
	} else {
		/* c == ',' so we must do something(!) */
//		PUTCHAR('\n');
		switch( _rv.respState ) {
			case rsINIT: { /* cmd just received, change state to CMD, and store CMD action */
				uint8_t n;
				char *ch2=resp;

					STRTOD(n, ch2);	/* convert resp to n */
					if((n >= 0) && (n <= raSTOP)) {
						_rv.respAction = n;
						_rv.respState = rsCMD;
					}
					memset(resp, 0, sizeof(resp));
			};
			return (true); 
			break;
			
			case rsCMD: {	/* payload received, so execute command with given payload */
				/* payload is in resp */
//				PRINTF( "CMD: ");
				switch( _rv.respAction ) {
					case raSETNODE: /* set node */
						PRINTF("(1) set node number: "); PRINTFn( resp );
						break; 
					case raSETDATASERVER: 	/* set data server */
						PRINTF("(2) set data server: "); PRINTFn( resp );
						break;
					case raSETSERVERPORT:	/* set server port */
						PRINTF("(3) set server port: "); PRINTFn( resp );
						break;
					case raSETAPN:		/* set APN */
						PRINTF("(4) set APN: "); PRINTFn( resp );
						break;
					case raSETUSER:			/* set USER */
						PRINTF("(5) set USER: "); PRINTFn( resp );
						break;
					case raSETPASS:			/* set PASS */
						PRINTF("(6) set PASS: "); PRINTFn( resp );
						break;
					case raSETLOGPROG:		/* set log sampling program */
						PRINTF("(7) set LOG program: "); PRINTFn( resp );
						{
							char *ch2 = resp;
							PRINTF("hour: ");
							while( *ch2 && *ch2 != ':') PUTCHAR( *ch2++ );
							ch2++;
							PRINTF("\nminute: ");
							while( *ch2 && *ch2 != ':') PUTCHAR( *ch2++ );
							ch2++;
							PRINTF("\nday: ");
							while( *ch2 && *ch2 != ':') PUTCHAR( *ch2++ );
							PUTCHAR('\n');
						}
						break;
					case raSETNETPROG:		/* set net sampling program */
						PRINTF("(8) set NET program: "); PRINTFn( resp );
						{
							char *ch2 = resp;
							PRINTF("hour: ");
							while( *ch2 && *ch2 != ':') PUTCHAR( *ch2++ );
							ch2++;
							PRINTF("\nminute: ");
							while( *ch2 && *ch2 != ':') PUTCHAR( *ch2++ );
							ch2++;
							PRINTF("\nday: ");
							while( *ch2 && *ch2 != ':') PUTCHAR( *ch2++ );
							PUTCHAR('\n');
						}
						break;
					case raSETAPIKEY:		/* set APIKEY */
						PRINTF("(9) set APIKEY: "); PRINTFn( resp );
						break;
					case raSETACTIVE:		/* set ACTIVE flag, NOTE: This will disable the module */
						PRINTF("(a) set ACTIVE flag: "); PRINTFn( resp );
						break;
					case raSETDST:			/* set DST flag */
						PRINTF("(b) set DST flag: "); PRINTFn( resp );
						break;
					case raSETDNS:			/* set DNS flag, use DNS resolver for URLs */
						PRINTF("(c) set DNS flag: "); PRINTFn( resp );
						break;
					case raSETRTC:			/* set RTC */
						PRINTF("(d) set RTC: "); PRINTFn( resp );
						break;
					case raSTOP:			/* STOP response parsing */
						PRINTFn("(f) STOP");
						return (false);
						break;
				default:
					return (false);
				}
				_rv.respState = rsINIT;
				_rv.respAction = raNONE;
				memset(resp, 0, sizeof(resp));
			}; 
			return (true);
			break;
		}
	}
  return (false);
}
