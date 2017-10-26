#!/bin/sh

VER="ver.h"
VERIN="ver.h.in"

VERSION=`git log | head -1 - | gawk '{ print  } ' -  | tail -c 8 -`

echo version: $VERSION


cat $VERIN | sed s/GITVERSION/$VERSION/g - > $VER

