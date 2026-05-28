# Makefile to compile each .c file into its own executable

<<<<<<< Updated upstream
CFLAGS = -Wall -Wextra -Werror --std=c99
=======
CFLAGS = -Wall -Wextra --std=c99 -g
>>>>>>> Stashed changes

SRC = $(wildcard *.c)
EXE = $(SRC:.c=)

all: $(EXE)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(EXE)

.PHONY: all clean
