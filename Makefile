CC=gcc
CFLAGS=
LDFLAGS=-lrt -lpthread

EXE=get_sample
OBJECTS=get_sample.c

client: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXE) $(LDFLAGS)

all:client

.PHONY: clean

clean:
	rm -f *~ *.o $(EXE)
