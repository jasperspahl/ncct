CC=gcc
CFLAGS=-Wall -Wextra -g -std=c11 -pedantic
LDFLAGS=-lncurses

.PHONY: all
all: ncct run

ncct: main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o ncct main.c

.PHONY: run
run: ncct
	./ncct

.PHONY: clean
clean:
	rm ncct
