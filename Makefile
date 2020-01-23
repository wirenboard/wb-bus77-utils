
CC = gcc
TOOLCHAIN := arm-linux-gnueabihf-
TPATH := ~/wb/wb-cross/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/

SRC = $(wildcard *.c)
CFLAGS = -Wall
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.c=.o)))
BUILD_DIR = build
TARGET = frame

WB = root@wirenboard-AQRLISBO.local

all: $(TARGET) send

$(TARGET): $(OBJECTS)
	$(TPATH)$(TOOLCHAIN)$(CC) $(CFLAGS) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: %.c Makefile $(BUILD_DIR)
	@echo ---$@
	$(TPATH)$(TOOLCHAIN)$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)

send:
	$(shell scp $(TARGET) $(WB):)

$(BUILD_DIR):
	mkdir -p $@
