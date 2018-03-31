CC=gcc
SRC=$(wildcard src/*.c)
MAIN=bin/fs
CFLAGS=-m32 -Iinclude -Wall


# BUILD FUNCTION
build:
	@mkdir -p bin
	@$(CC) $(CFLAGS) $(SRC) -o $(MAIN)

clean:
	@rm bin/*

run:
	$(MAIN)