CC = gcc
CFLAGS = -Wall -std=c99 -pedantic -g3

all: Blotto GmapUnit

Blotto: blotto.o gmap.o entry.o string_key.o
		${CC} -o ${{CFLAGS} $@ $^ -lm

GmapUnit: gmap.o gmap_unit.o gmap_test_functions.o string_key.o
		${CC} -o ${{CFLAGS} $@ $^ -lm

blotto.o: gmap.h entry.h string_key.h

gmap.o: gmap.h string_key.h

gmap_unit.o: gmap_test_functions.h gmap.h string_key.h

gmap_test_functions.o: gmap_test_functions.h gmap.h string_key.h

entry.o: entry.h

string_key.o: string_key.h