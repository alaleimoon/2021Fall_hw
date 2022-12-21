CC = gcc
CFLAGS = -Wall -std=c99 -pedantic -g3

all: GPS TrackUnit

GPS: gps.o track.o location.o trackpoint.o more_math.o
		${CC} -o ${{CFLAGS} $@ $^ -lm

TrackUnit: track_unit.o track.o location.o trackpoint.o more_math.o
		${CC} -o ${{CFLAGS} $@ $^ -lm

gps.o: track.h

track_unit.o: track.h

track.o: track.h more_math.h location.h

location.o: location.h

trackpoint.o: trackpoint.h

more_math.o: more_math.h