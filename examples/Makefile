# Makefile to compile each .c file into its own executable

CC = gcc
CFLAGS = -Wall -Wextra --std=c99

SRC = $(wildcard *.c)
EXE = $(SRC:.c=)

all: $(EXE)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXE)

.PHONY: all clean
