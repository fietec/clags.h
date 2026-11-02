TARGET = tests
SRCS   = tests.c syntax.c parsing.c validation.c
OBJS   = $(SRCS:%.c=build/%.o)

BUILD_OBJ = build/building.o
NOB_OBJ   = build/nob.o
HEADER    = ../clags.h

CFLAGS          = -Wall -Wextra -Werror -std=c99 -g
PEDANTIC_CFLAGS = $(CFLAGS) -pedantic
NOBCFLAGS       = $(CFLAGS) -xc -DNOB_IMPLEMENTATION

$(TARGET): $(OBJS) $(BUILD_OBJ) $(NOB_OBJ)
	$(CC) $(PEDANTIC_CFLAGS) $(OBJS) $(BUILD_OBJ) $(NOB_OBJ) -o $(TARGET)

build/%.o: %.c $(HEADER)
	@mkdir -p build
	$(CC) $(PEDANTIC_CFLAGS) -c $< -o $@

$(BUILD_OBJ): building.c $(HEADER) nob.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

$(NOB_OBJ): nob.h
	@mkdir -p build
	$(CC) $(NOBCFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf build $(TARGET)
