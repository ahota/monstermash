CC=gcc
CFLAGS=-Wall
SOURCES=monster_mash.c
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) mmash

mmash: $(OBJECTS)
    $(CC) $(OBJECTS) -o

monster_mash.o: monster_mash.c
    $(CC) $(CFLAGS) monster_mash.c

clean: rm *.o mmash
