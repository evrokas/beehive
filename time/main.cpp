#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "time.h"



#define __putchar(c)		Serial.write(c)
#define STRTOD(_v,_s)	_v=0;while(isdigit( *_s )) { _v=_v*10+(*_s - '0');_s++; }



/*
 * parameter format:
 * min , hour , day
 * min: n1,n2,n3,...   / n1-n2[,n3-n4]|n1-|-n2    / n1/a | n1-n2/a,[n3-n4/b] |
 */
 

timekeeping_t tk;


/* atyp:
 * 	0		hour
 *	1		minute
 *	2		day
 *
 */

 
char *parsestring(timekeeping_t &tk, uint8_t atyp, char *ch)
{
	uint8_t d;
		
#ifdef DEBUG
		switch(atyp) {
			case 0:	printf("hours\n"); break;
			case 1: printf("minutes\n"); break;
			case 2: printf("days\n"); break;
		}
#endif

		while(strlen(ch)) {
			if(*ch == '*') {
				/* * notation */
				d = 1;
					
				ch++;
				if(*ch == '/') {
					/*  * /n notation */
					ch++;	/* number after / */
					STRTOD(d,ch);	/* steps */
				}
				
				
				switch(atyp) {
					case 0: for(int i=0;i<24;i+=d)initTime(tk, i, -1, -1);break;
					case 1: for(int i=0;i<60;i+=d)initTime(tk, -1, i, -1);break;
					case 2: for(int i=0;i<7;i+=d)initTime(tk, -1, -1, i); break;
				}
			}
				
			if(isdigit(*ch)) {
				/* a notation */
				STRTOD(d, ch);

#ifdef DEBUG
				printf("%d\n", d);			
#endif

				if(*ch == '-') {
				/* a-b notation */
					uint8_t dd, nn=1;
						
						ch++;
						STRTOD(dd,ch)

#ifdef DEBUG
						printf("range notation: %d - %d\n", d, dd);
#endif
	
						if(*ch == '/') {
							/* a-b/n notation */
							ch++;
							STRTOD(nn,ch);
						}
						
						switch(atyp) {
							case 0: for(int i=d;i<=dd;i+=nn)initTime(tk, i, -1, -1);break;
							case 1: for(int i=d;i<=dd;i+=nn)initTime(tk, -1, i, -1);break;
							case 2: for(int i=d;i<=dd;i+=nn)initTime(tk, -1, -1, i); break;
						}

				} else {
					switch(atyp) {
						case 0: initTime(tk, d, -1, -1);break;
						case 1: initTime(tk, -1, d, -1);break;
						case 2: initTime(tk, -1, -1, d); break;
					}
				}
			}
				
			if(*ch == ',')ch++;
			
			if(*ch == ' ') {
				ch++;
				return (ch);
			}
		}
		
	return (ch);
}



char output;			/* 'j' json, 'x' hexdecimal */

int main(int argc, char *argv[])
{
 int i;
 char *ch;

//		printf("encode time in crontab format\n");

//		printf("Size of timekeeping_t: %ld bytes\n", sizeof( timekeeping_t ) );

#if 0
		clearTime( tk );
	
		initTime(tk, 12, 45);
		printTime( tk );
		
		
		initTime(tk, 12, 46);
		printTime( tk );
	
		initTime(tk, 23, 59);
		printTime( tk );
#endif
	

		clearTime(tk);

		i = 0;		
		while(i < argc) {
			ch = argv[++i];

			if(ch[0] == '-') {
				switch(ch[1]) {
					case 'j':	/* print out json data */
						output = 'j';
//						printf("json output\n");
						break;
					case 'x': /* print out hexadecimal data */
						output = 'x';
//						printf("hexadecimal output\n");
						break;
					case 'X':
						output = 'X';
						break;
					case 'b':	/* print out binary data */
						output = 'b';
						break;
				}
			} else {
				ch = parsestring(tk, 0, ch);
				ch = parsestring(tk, 1, ch);
				ch = parsestring(tk, 2, ch);
			
				switch(output) {
					case 'x': printTime( tk ); break;
					case 'X': printTime( tk, 1 ); break;
					case 'j': printTimeJSON( tk ); break;
					case 'b': printTimeBinary( tk ); break;
				}
				
					
				return 0;
			}
		}
		
		
		printf("Check time (2,43,1): %s\n", checkTime(tk, 2, 43,1)?"ok":"not ok");
		
  return 0;
}
