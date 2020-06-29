#include <stdio.h>
#include <getopt.h>
#include "nmisc.h"
#include "can_bus.h"
#include "bus77_en_ch.h"
#include "modbus_frame.h"

uint8_t modbus_buffer[256];

int main(int argc, char *argv[])
{
    if (argc == 1) {
        printf(
            "Welcome to Wiren Board bus77-modbus client\n"
            "   -d   can device (default can0)\n"
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
    int modbus_func = 3;
    int modbus_reg = 128;
    int modbus_amount = 1;

    int c;
    int args_amount = 1;
    while ((c = getopt(argc, argv, "d:a:t:r:c:")) != -1) {
        args_amount += 2;
        switch (c) {
        case 'd':
            device = optarg;
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

    can_bus_init(device);

    printf("[open channel]\n");
    bus77_open_channel();

    int len = modbus_make_frame(modbus_id, modbus_func, modbus_reg, vals, modbus_amount, modbus_buffer);
    if (len == 0) {
        printf("Wrong modbus function\n");
        return 1;
    }

    printf("modbus frame > : ");
    dp_hb8(modbus_buffer, len);
    putchar('\n');
    bus77_send_modbus_frame(modbus_buffer, len);

    len = bus77_recieve_modbus_frame(modbus_buffer);
    if (len == 0) {
        printf("ERROR occured!\n");
        return 1;
    }
    printf("modbus frame < : ");
    dp_hb8(modbus_buffer, len);
    putchar('\n');
    modbus_parse_answer(modbus_buffer, len);

    printf("[close channel]\n");
    bus77_close_channel();

    return 0;
}
