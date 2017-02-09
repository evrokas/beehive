# Makefile for Beehive Monitoring System
#
# (c) Evangelos Rokas, 2016, 2017
#

BOARD_TAG = bb8
ROOT_DIR	= ..

include Makefile.top


USER_LIB_PATH=$(ROOT_DIR)/beehive_libs


MONITOR_PORT = /dev/ttyUSB0
MONITOR_BAUDRATE = 9600

include	$(ARDMK_DIR)/Arduino.mk
