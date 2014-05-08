binaries=jackiappo metro
all: $(binaries)

CCOPTS=-Wall -ansi
CC=gcc $(CCOPTS)
SOURCES=jackiappo.c config_parse.c
OBJECTS=$(SOURCES:.c=.o)

jackiappo: $(OBJECTS)
	$(CC) -o $@ $^ `pkg-config --cflags --libs jack libconfig`

jackiappo.o: jackiappo.c jackiappo.h
	$(CC) -c -o $@ `pkg-config --cflags --libs jack` $<

config_parse.o: config_parse.c config_parse.h
	$(CC) -c -o $@ `pkg-config --cflags --libs libconfig` $<
#jackiappo.o: jackiappo.c
#	$(CC) -c -o $@ $^ `pkg-config --cflags --libs jack`

clean:
	rm -f $(OBJECTS) $(binaries) metro.o


metro: metro.c
	$(CC) -o $@ $^ `pkg-config --cflags --libs jack` -lm
