binaries=jackiappo check_conf metro
all: $(binaries)

CCOPTS=-Wall -ansi
CC=gcc $(CCOPTS)
SOURCES=jackiappo.c config_parse.c
OBJECTS=$(SOURCES:.c=.o)

jackiappo: $(OBJECTS)
	$(CC) -o $@ $^ `pkg-config --cflags --libs jack libconfig`

jackiappo.o: jackiappo.c
	$(CC) -c -o $@ `pkg-config --cflags --libs jack` $<

config_parse.o: config_parse.c
	$(CC) -c -o $@ `pkg-config --cflags --libs libconfig` $<
#jackiappo.o: jackiappo.c
#	$(CC) -c -o $@ $^ `pkg-config --cflags --libs jack`

clean:
	rm -f $(OBJECTS) $(binaries) metro.o


check_conf: config_parse.o run_config.o
	$(CC) -o $@ $^ `pkg-config --cflags --libs libconfig`

run_config.o: run_config.c
	$(CC) -c -o $@ `pkg-config --cflags --libs libconfig` $<

metro: metro.c
	$(CC) -o $@ $^ `pkg-config --cflags --libs jack` -lm
