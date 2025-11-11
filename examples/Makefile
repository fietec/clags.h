# Makefile to compile each .c file into its own executable

CC = gcc
CFLAGS = -Wall -Wextra -O2

# Find all .c files
SRC = $(wildcard *.c)
# Create a list of executables (strip the .c extension)
EXE = $(SRC:.c=)

# Default target: build all executables
all: $(EXE)

# Rule: each executable depends on its matching .c file
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# Clean up
clean:
	rm -f $(EXE)

.PHONY: all clean
