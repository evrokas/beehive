# parameters compilation utility
#
# author: Evangelos Rokas <evrokas@gmail.com> 2017
#

# compiler
CC	= gcc

# C flags
CFLAGS	= -Wall -g

# Linker flags
LFLAGS	=


CFILES	= main.c

OFILES	= main.o

BINARY	= scp

.c.o:
	$(CC) $(CFLAGS) -c $<

all: $(BINARY)

$(BINARY): $(OFILES)


claen