#!/bin/sh

#FSBLOCKS=2048

# create stats file system
dd if=/dev/zero of=filesystem.stats bs=4 count=$FSBLOCKS

# create main file system
dd if=/dev/zero of=filesystem bs=32 count=$FSBLOCKS
