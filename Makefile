
CC = gcc
TOOLCHAIN := arm-linux-gnueabihf-
TPATH := ~/wb/wb-cross/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/

SRC = can_bus.c crc16.c modbus.c bus77_en_ch.c nmisc.c

CFLAGS = -Wall
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.c=.o)))
BUILD_DIR = build
TARGET = frame

WB = root@wirenboard-AQRLISBO.local

all: $(TARGET) send

wb_b77_client: $(BUILD_DIR)/wb_b77_client.o $(OBJECTS)
	$(TPATH)$(TOOLCHAIN)$(CC) $(CFLAGS) $(OBJECTS) $(BUILD_DIR)/wb_b77_client.o -o $@

$(TARGET): $(BUILD_DIR)/TARGET.o $(OBJECTS)
	$(TPATH)$(TOOLCHAIN)$(CC) $(CFLAGS) $(OBJECTS) $(BUILD_DIR)/TARGET.o -o $@

$(BUILD_DIR)/%.o: %.c Makefile $(BUILD_DIR)
	@echo ---$@
	$(TPATH)$(TOOLCHAIN)$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)

send:
	$(shell scp $(TARGET) $(WB):)

$(BUILD_DIR):
	mkdir -p $@
