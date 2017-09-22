BOARD_TAG = bb8

ROOT_DIR	= ../..

include ../Makefile.top

USER_LIB_PATH=$(ROOT_DIR)/beehive_external_libs
#,$(ROOT_DIR)/beehive_external_libs/TinyGsm


MONITOR_PORT = /dev/ttyUSB0
MONITOR_BAUDRATE = 9600

include	$(ARDMK_DIR)/Arduino.mk

CPPFLAGS	+= -DGSM_SERIAL_BAUDRATE=9600 -DGSM_MODULE=1
