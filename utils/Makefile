# parameters compilation utility
#
# author: Evangelos Rokas <evrokas@gmail.com> 2017
#

# compiler
CC	= g++

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

$(OFILES): $(CFILES)

$(BINARY): $(OFILES)
	$(CC) $(CFLAGS) -o $@ $(OFILES) $(LFLAGS)


clean:
	rm -fv $(BINARY) $(OFILES)
