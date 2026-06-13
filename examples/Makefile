# Makefile to compile each .c file into its own executable

CC := gcc
CFLAGS := -Wall -Wextra -Werror --std=c99 -g -I..

SRC := $(shell find . -type f -name '*.c')
EXE := $(patsubst %.c,%,$(SRC))

all: $(EXE)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXE)

.PHONY: all clean
