#!/bin/sh
#
# Beehive Monitoring System - BMS
# (c) Evangelos Rokas, 2018
# 
# This software is Free Software and distributed under
# the terms of GNU General Public License.
#
###
# This is the pre-upload hool for various action prior
# to actually uploading the firmware, like versioning
###
#
#echo PRE-upload-HOOK
UPLOAD_FILE=".upload"
UPLOAD_INFO_FILE="upload.info"


UPNUM0=`cat $UPLOAD_FILE`
UPNUM=`expr $UPNUM0 + 1`
echo $UPNUM > $UPLOAD_FILE

echo "===================" >> $UPLOAD_INFO_FILE
echo "New upload # $UPNUM" >> $UPLOAD_INFO_FILE
grep 'git version string' ver.h >> $UPLOAD_INFO_FILE
avr-size -C --mcu=atmega328p build-bb8/beehive.elf | grep 'Program' - >> $UPLOAD_INFO_FILE
avr-size -C --mcu=atmega328p build-bb8/beehive.elf | grep 'Data' - >> $UPLOAD_INFO_FILE

