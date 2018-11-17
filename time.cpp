/*
 * time.cpp - time keeping functions
 *
 * BEEWatch - Beehive Monitoring System
 * (c) Evangelos Rokas, 2015-18
 *
 * This software is Free Software and distributed under
 * the terms of GNU General Public License.
 *
 * $version$
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bms.h"
#include "time.h"

void clearTime(timekeeping_t &tk)
{
	memset(&tk, 0, sizeof(tk));
}

bool initTime(timekeeping_t &tk, uint8_t hr, uint8_t min, uint8_t dy)
{
//	printf("setting h:%d m:%d d:%d\n", hr, min, dy);
	
	if((hr >= 0) && (hr <= 23))
		SetBit(tk.hr, hr);

	if((min >= 0) && (min <= 59))
		SetBit(tk.min, min);

#ifdef DAYS_IN_TIME
	if((dy >= 0) && (dy <= 6))
		SetBit(tk.day, dy);
#endif

	return true;
}

bool checkTime(timekeeping_t &tk, uint8_t hr, uint8_t min, uint8_t dy)
{
	if((TestBit(tk.hr, hr))
		&& (TestBit(tk.min, min))
#ifdef DAYS_IN_TIME
		&& (TestBit(tk.day, dy))
#endif
	)return true;

	return false;
}


/* expect a string of the form:
 * ffffff:fffffffffffffff:7f
 * hour:minute:day
 * and convert it to timekeeping_t */
bool initHexTime(char *str, timekeeping_t &tk)
{
  uint8_t cb, cbit,
  		state=0;			/* 0 is for hour, 1 is for minute, 2 is for day */

  	cbit = 23;	/* current bit in process */
  	while( *str ) {
  		if(*str == ':') {
  			state++;		/* advance state */
  			str++;			/* advance position in str */
  			
  			if(state == 1)cbit = 59;
  			else if(state == 2)cbit = 7;
  			else return false;
  			continue;
  		} else

#if 0
	  	if(cbit < 0) {
  			printf("!!! error !!!");
  			return (false);
	  	}
#endif


  		if( (*str >= '0') && (*str <= '9'))cb = *str - '0';
  		else if((*str >= 'a') && (*str <= 'f'))cb = 10 + (*str - 'a');
  		else return (false);

  		for(int i=3;i>=0;i--) {
  			if(cb & (1<<i)) {
				switch(state) {
		  			case 0: initTime(tk, cbit, -1, -1); break;
		  			case 1: initTime(tk, -1, cbit, -1); break;
		  			case 2: initTime(tk, -1, -1, cbit); break;
		  			default: return (false);
		  		}  			
		  	}
		  	cbit--;
  		}
  		str++;
  	}
  return (true);
}

#if 0
bool findNextTime(timekeeping_t &tk, uint8_t hr, uint8_t min, uint8_t dy, uint8_t &hh, uint8_t &mm, uint8_t &dd)
{
  uint8_t h, m, d;

  	h = hr;
  	m = min+1;
  	d = dy;


		/* loop through all bits in the variables of 'tl' to see a bits are set */
		do {	/* days */

			if(TestBit(tk.day, d)) {
				do {	/* hours */

					if(TestBit(tk.hr, h)) {
						do {	 /* minutes */
					  	if(TestBit(tk.min, min)) {
					  		hh = h;
					  		mm = m;
#ifdef DAYS_IN_TIME
								dd = d;
#endif
								return (true);
							}
							m++;
						}	while(m < 60);
					}
					/* m = 60, zero m and increase h */
					m = 0;
					h++;
				} while(h < 60);
		
			}	
			h = 0;
			d++;
			if(d > 6) d = 0;
		} while(d != dy);
		
	return (false);
}
#endif


#ifdef LINUX_NATIVE_APP

void printTime(timekeeping_t &tk, uint8_t flags)
{
  int i;
  uint8_t d;
  char ch[3], str[64];
  
  	memset(str, 0, sizeof(str));
  	

  	d = 0;
  	for(i=23;i>=0;i--) {
  		d<<=1;

#ifdef DEBUG
  		PUTCHAR( (TestBit(tk.hr, i)?'1':'0') );
#endif

  		d |= TestBit(tk.hr, i)?1:0;
  		if(i % 8 == 0) {
#ifdef DEBUG
  			PUTCHAR(' ');PUTCHAR('<');
#endif
  			sprintf(ch, "%02x", d);
  			strcat(str, ch);
#ifdef	DEBUG
  			PUTCHARX(d);
			PUTCHAR('>'); PUTCHAR(' ');
#endif
			d=0;
		}
	}

	if(flags == 1)
		strcat( str, ":" );
	else
		strcat(str, " ");
		
#ifdef DEBUG
	PUTCHAR('\n');
#endif

	d=0;
  	for(i=59;i>=0;i--) {
  		d<<=1;

#ifdef DEBUG
  		PUTCHAR( (TestBit(tk.min, i)?'1':'0') );
#endif

  		d |= TestBit(tk.min, i)?1:0;
  		
  		if(i % 8 == 0) {
#ifdef DEBUG
  			PUTCHAR(' '); PUTCHAR('<');
#endif

  			sprintf(ch, "%02x", d);
  			strcat(str, ch);

#ifdef DEBUG
  			PUTCHARX(d);
  			PUTCHAR('>');PUTCHAR(' ');
#endif
			d=0;
		}
	}

	if(flags == 1)
		strcat( str, ":" );
	else
		strcat(str, " ");

#ifdef DEBUG
	PUTCHAR('\n');
#endif

	d=0;
  	for(i=6;i>=0;i--) {
  		d <<= 1;
#ifdef DEBUG
  		PUTCHAR( (TestBit(tk.day, i)?'1':'0') );
#endif

  		d |= TestBit(tk.day, i)?1:0;
  		
  		if(i % 8 == 0) {
#ifdef DEBUG
  			PUTCHAR(' '); PUTCHAR('<');
#endif

  			sprintf(ch, "%02x", d);
  			strcat(str, ch);

#ifdef DEBUG
  			PUTCHARX(d);
  			PUTCHAR('>'); PUTCHAR(' ');
#endif
			d=0;
		}
	}

#ifdef DEBUG
		PUTCHAR('\n');
#endif

	/* this is a workaround to resolve a bug earlier in the code
	 * when printing hexadecimal numbers with %02x directive.
	 * The first zero at the minute field is always zero, therefore
	 * it is omitted, but when printf is used it is dumped, so a
	 * workaround is to move the rest of the string on top of the
	 * leading zero(!) */

	char *c = strchr(str, ':')+1;
	strcpy(c, c+1);
	
	printf("%s\n", str);
}

/*
 * JSON output
 * {min: {d: [0x11, 0x22, ..., 0x88]},
 *	hour: 0x11},
 *	day: 0x11}
 * }
 */

void printTimeJSON(timekeeping_t &tk)
{
  int i;
  uint8_t d;
  char str[64];
  
  	memset(str, 0, sizeof(str));
  	
  	PUTCHAR("{");
  	PUTCHAR("\"hour\":[");
  	
  	
  	d = 0;
  	for(i=23;i>=0;i--) {
  		d<<=1;

#ifdef DEBUG
  		PUTCHAR( (TestBit(tk.hr, i)?"1":"0") );
#endif

  		d |= TestBit(tk.hr, i)?1:0;
  		if(i % 8 == 0) {
  			PUTCHAR("{\"d\":");
  			PUTCHAR2(d);
			PUTCHAR("}");
			if(i != 0)PUTCHAR(",");
			d=0;
		}
	}

	PUTCHAR("],\"min\":[");

	d=0;
  	for(i=59;i>=0;i--) {
  		d<<=1;

#ifdef DEBUG
  		PUTCHAR( (TestBit(tk.min, i)?"1":"0") );
#endif

  		d |= TestBit(tk.min, i)?1:0;
  		
  		if(i % 8 == 0) {
  			PUTCHAR("{\"d\":");
  			PUTCHAR2(d);
			PUTCHAR("}");
			if(i != 0)PUTCHAR(",");
			d=0;
		}
	}

	PUTCHAR("],\"day\":");

	d=0;
  	for(i=6;i>=0;i--) {
  		d <<= 1;
#ifdef DEBUG
  		PUTCHAR( (TestBit(tk.day, i)?"1":"0") );
#endif

  		d |= TestBit(tk.day, i)?1:0;
  		
  		if(i % 8 == 0) {
  			PUTCHAR2(d);
			d=0;
		}
	}

		PUTCHAR("}\n");
}
#undef DEBUG

#endif		/* LINUX_NATIVE_APP */


void printTimeBinary(timekeeping_t &tk)
{
  int i;
  uint8_t d;
  
  	d = 0;
  	for(i=23;i>=0;i--) {
  		d<<=1;
  		PUTCHAR( (TestBit(tk.hr, i)?"1":"0") );

  		d |= TestBit(tk.hr, i)?1:0;
  		if(i % 8 == 0) {
				d=0;
			}
		}

		PUTCHAR(" ");
		
		d=0;
  	for(i=59;i>=0;i--) {
  		d<<=1;
  		PUTCHAR( (TestBit(tk.min, i)?"1":"0") );

  		d |= TestBit(tk.min, i)?1:0;
  		
  		if(i % 8 == 0) {
				d=0;
			}
		}

		PUTCHAR(" ");
		

		d=0;
  	for(i=6;i>=0;i--) {
  		d <<= 1;
  		PUTCHAR( (TestBit(tk.day, i)?"1":"0") );

  		d |= TestBit(tk.day, i)?1:0;
  		
  		if(i % 8 == 0) {
				d=0;
			}
		}

		PUTCHAR("\n");
}
