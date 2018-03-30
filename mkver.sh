#!/bin/sh

VER="ver.h"
VERIN="ver.h.in"

VERSTR=`git log | head -1 - | gawk '{ print } ' -`

#VERSION=`git log | head -1 - | gawk '{ print  } ' -  | tail -c 9 -`
VERSION=`echo $VERSTR | tail -c 9 -`
VERSION_SHORT=`echo $VERSTR | tail -c 5 -`

echo version: $VERSION
echo version short: $VERSION_SHORT

cat $VERIN | sed s/GITVERSION/$VERSION/g - | sed s/GITSHORTVERSION/$VERSION_SHORT/g - > $VER

