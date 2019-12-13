
CC = gcc

SRC = $(wildcard *.c)
CFLAGS=-c -Wall
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.c=.o)))
BUILD_DIR = build
TARGET = frame

all: $(TARGET) test

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: %.c Makefile $(BUILD_DIR)
	@echo ---$@
	$(CC) $(CFLAGS) $< -o $@

test: $(TARGET)
	./$(TARGET)

$(BUILD_DIR):
	mkdir -p $@
