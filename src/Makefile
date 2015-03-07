CC=gcc
CFLAGS=-c -g
SOURCES_SHARED=input.c
SOURCES_SERVER=monster_mash.c tools.c $(SOURCES_SHARED)
SOURCES_CLIENT=mm_client.c $(SOURCES_SHARED) #client.c $(SOURCES_SHARED)
OBJECTS_SERVER=$(SOURCES_SERVER:.c=.o)
OBJECTS_CLIENT=$(SOURCES_CLIENT:.c=.o)

all: mmash_s mmash_c

mmash_s: $(SOURCES_SERVER) mmash

mmash_c: $(SOURCES_CLIENT) client

mmash: $(OBJECTS_SERVER)
	$(CC) $(OBJECTS_SERVER) -o $@

client: $(OBJECTS_CLIENT)
	$(CC) $(OBJECTS_CLIENT) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean: 
	rm *.o mmash client
