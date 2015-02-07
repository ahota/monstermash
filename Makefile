CC=gcc
CFLAGS=-Wall -c

all: mmash

mmash: monster_mash.o
	$(CC) monster_mash.o -o mmash

monster_mash.o: monster_mash.c
	$(CC) $(CFLAGS) monster_mash.c

clean: 
	rm *.o mmash
