# Makefile to compile each .c file into its own executable
CC := gcc
CFLAGS = -Wall -Wextra -Werror -pedantic --std=c99 -I..

SRC = $(wildcard *.c)
EXE = $(SRC:.c=)

all: $(EXE)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXE)

.PHONY: all clean
