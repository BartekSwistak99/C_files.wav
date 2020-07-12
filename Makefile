ProgramName=loadWAV
CCFLAGS= -g -Wall -pedantic
CC=gcc
LDFLAGS= -lm `sdl2-config --cflags --libs` 
src=$(wildcard *.c) #get all files .c
obj=$(patsubst %.c,%.o,$(src)) #change *.c strings to *.o

all: $(ProgramName)

%.o:%.c
	$(CC) -c $(CCFLAGS) $< -o $@
$(ProgramName): $(obj)
	$(CC) $^ -o $@ $(LDFLAGS) 
clean:
	-rm -f *.o $(ProgramName)
run: 
	./$(ProgramName) test2.wav
