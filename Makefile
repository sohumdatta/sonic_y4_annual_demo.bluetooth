CC=gcc
CFLAGS=
LDFLAGS=-lrt -lpthread

EXE=get_sample
OBJECTS=get_sample.c
MEXA_OBJECTS=get_sample_mex.c

client: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXE) $(LDFLAGS)

mexa: $(MEXA_OBJECTS) create_mex.m
	matlab -nodisplay -nosplash < create_mex.m
	
all:client mexa

.PHONY: clean

clean:
	rm -f *~ *.o $(EXE)
