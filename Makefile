binaries=jackiappo metro
all: $(binaries)

CCOPTS=-Wall -std=c99 -pedantic
CC=gcc $(CCOPTS)
ifeq ($(DEBUG),1)
CCOPTS += -g
endif
SOURCES=jackiappo.c config_parse.c pipe.c rules.c
OBJECTS=$(SOURCES:.c=.o)

jackiappo: $(OBJECTS)
	$(CC) -o $@ $^ `pkg-config --cflags --libs jack libconfig`

rules.o: rules.c rules.h
pipe.o: pipe.c pipe.h
	$(CC) -c -o $@ $<

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
