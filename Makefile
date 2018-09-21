# Makefile for Beehive Monitoring System
#
# BeeWatch Project
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


# add conditional for ver.h version header
#ver.h: ver.h.in
#	./mkver.sh

#CXXFLAGS_STD	= -std=c++0x

AVR_SIZE_OPTS	= -t d


include	$(ARDMK_DIR)/Arduino.mk

CPPFLAGS	+= -DGSM_SERIAL_BAUDRATE=9600				# -DNEOSW_RX_BUFFER_SIZE=64
CPPFLAGS	+= -DNEOSWSERIAL_EXTERNAL_PCINT

CPPFLAGS	+= $(SRVURL)
CPPFLAGS	+= $(SRVPORT)
AVRDUDE_OPTS=-V -v -v

#CXXFLAGS	+= -felide-constructors

pmon:
	putty -load serial


git-upl:
	git commit -a -m "upload commit (automatic)"
	$(MAKE) upl
		

upl:
	./mkver.sh
	$(MAKE) upload


.PHONY: git-upl upl pmon
