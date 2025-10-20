TARGET = example
SRC_FILES = example.c
CFLAGS = -Wall -Wextra

$(TARGET): $(SRC_FILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC_FILES)
