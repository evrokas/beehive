/*
 * error.cpp - error handling functionality
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


#include <stdio.h>

#include <Arduino.h>

#include "bms.h"
#include "error.h"


char __error_buffer[ ERROR_BUFFER_LENGTH ];
uint16_t __error_head;
uint16_t __error_tail;

void initErrorHandling()
{
	__error_head = 0;
	__error_tail = 0;
	memset( __error_buffer, 0, sizeof( __error_buffer ));
}

void doneErrorHandline()
{

}

bool logError(uint8_t erType)
{
	return (true);
}