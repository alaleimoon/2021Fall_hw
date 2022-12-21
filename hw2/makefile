CC = gcc
CFLAGS = -Wall -std=c99 -pedantic -g3

TSP: tsp.o location.o
		${CC} -o ${{CFLAGS} $@ $^ -lm

tsp.o: location.h

location.o: location.h