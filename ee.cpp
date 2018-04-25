#ifndef __EE_H__
#define __EE_H__

#if LINUX_NATIVE_APP == 1


#else

#include <Arduino.h>
#include <Wire.h>

#endif


#include "mem.h"
//#include "data.h"


extern uint8_t	__ee_dev_addr;
extern uint32_t	__ee_dev_size;


/* set this to 1 to use dummy routines that do not actually
 * write or read anything from EEPROM, otherwise set to 0 */
#define USE_DUMMY_ROUTINES	0


#if LINUX_NATIVE_APP == 1

/* compile for linux native application, used for testing
 * functions */

#include "drivers/ee_linux.hpp"

#else

/* compile for beehive production board
 */

#if USE_DUMMY_ROUTINES == 1
#  include "drivers/ee_dummy.hpp"
#else
#  include "drivers/ee_i2c.hpp"
#endif

#endif	/* LINUX_NATIVE_APP */

#endif	/* __EE_H__ */
