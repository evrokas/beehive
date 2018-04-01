#!/bin/sh

# create stats file system
dd if=/dev/zero of=filesystem.stats bs=4 count=2048

# create main file system
dd if=/dev/zero of=filesystem bs=32 count=2048
