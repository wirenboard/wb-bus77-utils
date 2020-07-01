
CC = gcc
TOOLCHAIN := arm-linux-gnueabihf-
TPATH := ~/wb/wb-cross/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/

SRC = can_bus.c crc16.c modbus.c bus77_en_ch.c debug_print.c

CFLAGS = -Wall
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.c=.o)))
BUILD_DIR = build

all: ./wb_b77_client ./wb_b77_flasher send

wb_b77_client: $(BUILD_DIR)/wb_b77_client.o $(OBJECTS)
	$(TPATH)$(TOOLCHAIN)$(CC) $(CFLAGS) $(OBJECTS) $(BUILD_DIR)/wb_b77_client.o -o $@

wb_b77_flasher: $(BUILD_DIR)/wb_b77_flasher.o $(OBJECTS)
	$(TPATH)$(TOOLCHAIN)$(CC) $(CFLAGS) $(OBJECTS) $(BUILD_DIR)/wb_b77_flasher.o -o $@

$(BUILD_DIR)/%.o: %.c Makefile $(BUILD_DIR)
	@echo ---$@
	$(TPATH)$(TOOLCHAIN)$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

send: wb_b77_client wb_b77_flasher
	scp $^ root@$(wb6):
