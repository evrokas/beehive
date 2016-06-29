BOARD_TAG = bb8
USER_LIB_PATH=../libraries

#initialize programmer params
ISP_PORT	= /dev/ttyACM0
ISP_PROG	= stk500v1
AVRDUDE_ISP_BAUDRATE	= 19200

ARDUINO_PORT = /dev/ttyACM0
ARDUINO_DIR = ../arduino-1.6.8
#ALTERNATE_CORE = avr
ARDMK_DIR = ../Arduino-Makefile

MONITOR_PORT = /dev/ttyUSB0
MONITOR_BAUDRATE = 9600

include	$(ARDMK_DIR)/Arduino.mk
