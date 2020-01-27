#include <stdio.h>
#include <getopt.h>
#include "nmisc.h"
#include "can_bus.h"
#include "modbus.h"
#include "bus77_en_ch.h"


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

    uint16_t vals[128];
    int vals_amount = 0;
    for (size_t i = optind; i < argc; i++) {
        sscanf(argv[i], "%d", &vals[vals_amount]);
        vals_amount++;
    }

    printf("args %d\n", vals_amount);
    dp_hb16(vals, vals_amount);

    // printf("\nParsed: dev: %s, id: %d, func: %d, addr: %d, count: %d\n", device, modbus_id, modbus_func, modbus_reg, modbus_amount);

    can_bus_init(device);

    printf("[open channel]\n");
    bus77_open_channel();

    int len;
    switch (modbus_func) {
    case MODBUS_CMD_READ_COILS: len = modbus_read_regs(modbus_id, MODBUS_CMD_READ_COILS, modbus_reg, modbus_amount); break;
    case MODBUS_CMD_READ_DISCRETE_INPUTS: len = modbus_read_regs(modbus_id, MODBUS_CMD_READ_DISCRETE_INPUTS, modbus_reg, modbus_amount); break;
    case MODBUS_CMD_READ_HOLDING_REGISTERS: len = modbus_read_regs(modbus_id, MODBUS_CMD_READ_HOLDING_REGISTERS, modbus_reg, modbus_amount); break;
    case MODBUS_CMD_READ_INPUT_REGISTERS: len = modbus_read_regs(modbus_id, MODBUS_CMD_READ_INPUT_REGISTERS, modbus_reg, modbus_amount); break;
    case MODBUS_CMD_WRITE_SINGLE_COIL: len = modbus_write_single_coil(modbus_id, modbus_reg, vals[0]); break;
    case MODBUS_CMD_WRITE_SIGLE_REGISTER: len = modbus_write_single_reg(modbus_id, modbus_reg, vals[0]); break;
    case MODBUS_CMD_WRITE_MULTIPLE_COIL: len = modbus_write_multiple_coil(modbus_id, modbus_reg, vals, vals_amount); break;
    case MODBUS_CMD_WRITE_MULTIPLE_REGISTER: len = modbus_write_multiple_regs(modbus_id, modbus_reg, vals, vals_amount); break;
    default:
        printf("Wrong modbus function\n");
        return 1;
        break;
    }

    bus77_send_modbus_frame(modbus_buf, len);

    len = bus77_recieve_modbus_frame(modbus_buf);
    printf("modbus frame: ");
    dp_hb8(modbus_buf, len);
    putchar('\n');
    modbus_parse_answer(len);

    printf("[close channel]\n");
    bus77_close_channel();

    return 0;
}
