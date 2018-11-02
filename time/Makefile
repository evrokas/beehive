COMMONFLAGS	= -DLINUX_NATIVE_APP


CC 			= gcc
CFLAGS	= $(COMMONFLAGS) -Wall -g

CXX			= g++
CXXFLAGS= $(COMMONFLAGS) -Wall -g


BIN = gentime

CFILES	= main.cpp time.cpp
OFILES	= main.o time.o

.c.o:
	$(CC) $(CFLAGS) -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<


$(BIN): $(OFILES) $(CFILES)
	$(CC) $(CFLAGS) $(OFILES) -o $(BIN)
		
clean:
	rm -f $(OFILES) $(BIN)
	