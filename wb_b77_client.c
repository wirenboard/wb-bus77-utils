#include <stdio.h>
#include <getopt.h>
#include "debug_print.h"
#include "can_bus.h"
#include "bus77_en_ch.h"
#include "modbus_frame.h"

uint8_t modbus_buffer[256];

int main(int argc, char *argv[])
{
    printf("Welcome to Wiren Board bus77-modbus client.\nVersion: " VERSION "\n");

    if (argc == 1) {
        printf(
            "   -d   can device (default can0)\n"
            "   -D   debug level (0) none, (1) modbus, (2) modbus + bus77)\n"
            "   -a   modbus slave addr (default 1)\n"
            "   -t   modbus function\n"
            "        (1) Read Coils, (2) Read Discrete Inputs, (5) Write Single Coil\n"
            "        (3) Read Holding Registers, (4) Read Input Registers, (6) WriteSingle Register\n"
            "        (15) WriteMultipleCoils, (16) Write Multiple register\n"
            "   -r   start reg addr\n"
            "   -c   regs count\n"
        );

        return 0;
    };

    char * device = "can0";
    int modbus_id = 1;
    int modbus_func = MODBUS_CMD_READ_HOLDING_REGISTERS;
    int modbus_reg = 128;
    int modbus_amount = 1;
    int debug_level = 0;

    int c;
    int args_amount = 1;
    while ((c = getopt(argc, argv, "d:a:t:r:c:D:")) != -1) {
        args_amount += 2;
        switch (c) {
        case 'd':
            device = optarg;
            break;
        case 'D':
            sscanf(optarg, "%d", &debug_level);
            break;
        case 'a':
            sscanf(optarg, "%d", &modbus_id);
            break;
        case 't':
            sscanf(optarg, "%d", &modbus_func);
            break;
        case 'r':
            sscanf(optarg, "%d", &modbus_reg);
            break;
        case 'c':
            sscanf(optarg, "%d", &modbus_amount);
            break;
        default:
            break;
        }
    }

    uint16_t vals[128] = {};
    int vals_amount = 0;
    for (size_t i = optind; i < argc; i++) {
        sscanf(argv[i], "%d", &vals[vals_amount]);
        vals_amount++;
    }
    if (modbus_func == MODBUS_CMD_WRITE_MULTIPLE_COIL || modbus_func == MODBUS_CMD_WRITE_MULTIPLE_REGISTER) {
        modbus_amount = vals_amount;
    }

    can_bus_init(device);
    if (debug_level > 1) {
        bus77_set_debug_level(1);
    }

    bus77_open_channel();

    int len = modbus_make_frame(modbus_id, modbus_func, modbus_reg, vals, modbus_amount, modbus_buffer);
    if (len == 0) {
        printf("Wrong modbus function\n");
        return 1;
    }

    if (debug_level > 0) {
        printf("modbus frame > : ");
        dp_hb8(modbus_buffer, len);
        putchar('\n');
    }
    bus77_send_modbus_frame(modbus_buffer, len);

    len = bus77_recieve_modbus_frame(modbus_buffer);
    if (len == 0) {
        printf("ERROR occured!\n");
        return 1;
    }
    if (debug_level > 0) {
        printf("modbus frame < : ");
        dp_hb8(modbus_buffer, len);
        putchar('\n');
    }
    int answ = modbus_parse_answer(modbus_buffer, len);

    bus77_close_channel();

    if (answ == 0) {
        return 1;
    }
    return 0;
}
