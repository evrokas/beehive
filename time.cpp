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


void printTime(timekeeping_t &tk)
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

		printf("HEX string: %s\n", str);
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
  		PUTCHAR( (TestBit(tk.hr, i)?'1':'0') );
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
  		PUTCHAR( (TestBit(tk.min, i)?'1':'0') );
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
  		PUTCHAR( (TestBit(tk.day, i)?'1':'0') );
#endif

  		d |= TestBit(tk.day, i)?1:0;
  		
  		if(i % 8 == 0) {
  			PUTCHAR2(d);
				d=0;
			}
		}

		PUTCHAR("}\n");
}


void printTimeBinary(timekeeping_t &tk)
{
  int i;
  uint8_t d;
//  char ch[3], str[64];
  
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
