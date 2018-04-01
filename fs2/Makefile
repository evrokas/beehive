# parameters compilation utility
#
# author: Evangelos Rokas <evrokas@gmail.com> 2017
#

# compiler
CC	= gcc
CXX	= g++

COMMON_FLAGS	= -DLINUX_NATIVE_APP=1	-DHAVE_DATA_H=1

# C flags
CFLAGS		= -Wall -g $(COMMON_FLAGS)
CXXFLAGS	= -Wall -g $(COMMON_FLAGS)

# Linker flags
LFLAGS	=


CFILES	= main.cpp data.cpp ../ee.cpp ../mem.cpp

OFILES	= main.o data.o ../ee.o ../mem.o

HFILES	= data.h ../bms.h ../mem.h


DEPS	= $(CFILES)	$(HFILES)

BINARY	= scp

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<


all: $(BINARY) 

$(OFILES): $(CFILES)

$(BINARY): $(DEPS) $(OFILES)
	$(CC) $(CFLAGS) -o $@ $(OFILES) $(LFLAGS)


clean:
	rm -fv $(BINARY) $(OFILES)
