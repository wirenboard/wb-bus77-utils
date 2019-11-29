
CC = gcc

all: frame test

frame: frame.c Makefile

	$(CC) -o $@ $<

test: frame
	./frame
