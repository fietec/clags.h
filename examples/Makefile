# Makefile to compile each .c file into its own executable

CFLAGS = -Wall -Wextra --std=c99

SRC = $(wildcard *.c)
EXE = $(SRC:.c=)

all: $(EXE)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXE)

.PHONY: all clean
