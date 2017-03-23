CC=gcc
CFLAGS=-std=c99 -Wall -pedantic

all: 2dxWavConvert 2dxDump 2dxBuild 2dxMerge

shared.o: shared.c shared.h
	$(CC) $(CFLAGS) -c shared.c -o shared.o

2dxWav.o: 2dxWav.c 2dxWav.h shared.o
	$(CC) $(CFLAGS) -c 2dxWav.c -o 2dxWav.o

2dxDump: 2dxDump.c 2dx.h shared.o
	$(CC) $(CFLAGS) 2dxDump.c shared.o -o 2dxDump

2dxBuild: 2dxBuild.c 2dx.h shared.o
	$(CC) $(CFLAGS) 2dxBuild.c shared.o -o 2dxBuild

2dxMerge: 2dxMerge.c 2dx.h shared.o
	$(CC) $(CFLAGS) 2dxMerge.c shared.o -o 2dxMerge

2dxWavConvert: 2dxWavConvert.c shared.o 2dxWav.o
	$(CC) $(CFLAGS) 2dxWavConvert.c shared.o 2dxWav.o -o 2dxWavConvert libsox-3.dll
	
.PHONY: clean
clean:
	rm -f *.o 2dxDump 2dxBuild 2dxMerge