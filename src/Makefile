CC=gcc
CFLAGS=-c -g
SOURCES=monster_mash.c tools.c
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) mmash

mmash: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm *.o mmash
