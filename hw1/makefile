CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -g

Statements: statements.o
		${CC} -o ${{CFLAGS} $@ $^

statements.o: statements.c
		${CC} -c ${{CFLAGS} $@ $^