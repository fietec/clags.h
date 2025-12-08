# Makefile to compile each .c file into its own executable

CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = $(wildcard *.c)
EXE = $(SRC:.c=)

all: $(EXE)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXE)

.PHONY: all clean
