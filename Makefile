# Makefile for Beehive Monitoring System
#
# (c) Evangelos Rokas, 2016, 2017
#


BOARD_TAG = bb8
ROOT_DIR	= ..

include Makefile.top


#USER_LIB_PATH=$(ROOT_DIR)/beehive_libs
USER_LIB_PATH=$(ROOT_DIR)/beehive_external_libs


MONITOR_PORT = /dev/ttyUSB0
MONITOR_BAUDRATE = 9600

#AVRDUDE_OPTS="-v -v"

# add conditional for ver.h version header
#ver.h: ver.h.in
#	./mkver.sh


include	$(ARDMK_DIR)/Arduino.mk

CPPFLAGS	+= -DGSM_SERIAL_BAUDRATE=9600 -DNEOSW_RX_BUFFER_SIZE=32
CPPFLAGS	+= $(SRVURL)
CPPFLAGS	+= $(SRVPORT)
