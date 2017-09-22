#ifndef __EE_H__
#define __EE_H__

#if LINUX_NATIVE_APP == 1


#else

#include <Arduino.h>
#include <Wire.h>

#endif


#include "mem.h"
#include "data.h"


extern uint8_t		__ee_dev_addr;
extern uint32_t	__ee_dev_size;


#if LINUX_NATIVE_APP == 1

/* compile for linux native application, used for testing
 * functions */

#include "drivers/ee_linux.hpp"

#else

/* compile for beehive production board
 */

#ifdef GSM_MODULE
// GSM_MODULE is defined in the Makefile of the gsm/ directory

#include "../drivers/ee_i2c.hpp"
#else
#include "drivers/ee_i2c.hpp"
#endif

#endif	/* LINUX_NATIVE_APP */

#endif	/* __EE_H__ */
