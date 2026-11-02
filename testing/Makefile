TARGET = tests
SRCS   = tests.c syntax.c parsing.c validation.c building.c
OBJS   = $(SRCS:%.c=build/%.o)
HEADER = ../clags.h

CFLAGS  = -Wall -Wextra -g

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

build/%.o: %.c $(HEADER)
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf build $(TARGET)
