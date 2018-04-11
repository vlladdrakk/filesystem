CC=gcc
SRC=$(wildcard src/*.c)
MAIN=bin/fs
CFLAGS=-m32 -Iinclude -Wall -g


# BUILD FUNCTION
build:
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(MAIN)
	@$(MAIN)

clean:
	rm bin/*

run:
	@$(MAIN)

debug:
	gdb $(MAIN)

