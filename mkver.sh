#!/bin/sh

VER="ver.h"
VERIN="ver.h.in"

VERSTR=`git log | head -1 - | gawk '{ print $2 } ' -`
UNUM=`cat .upload`

#VERSION=`git log | head -1 - | gawk '{ print  } ' -  | tail -c 9 -`
VERSION=`echo $VERSTR | head -c 9 -`
VERSION_SHORT=`echo $VERSTR | head -c 5 -`

echo version long: $VERSTR
#echo version: $VERSION
#echo version short: $VERSION_SHORT

cat $VERIN | sed s/GITLONGVERSION/$VERSTR/g - |  \
						sed s/GITVERSION/$VERSION/g - | \
						sed s/GITSHORTVERSION/$VERSION_SHORT/g - | \
						sed s/UPLOAD_NUM/$UNUM/g - > $VER
