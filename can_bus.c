#include "can_bus.h"
#include "debug_print.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <fcntl.h>
#include <time.h>

static int s;

static void print_frame(struct can_frame * f)
{
    uint32_t id = f->can_id & CAN_EFF_MASK;
    dp_h32(id);
    printf(" [%d] ", f->can_dlc);
    dp_hb8(f->data, f->can_dlc);
}

int can_bus_init(char * port_name)
{
    struct sockaddr_can addr;
    struct ifreq ifr;

    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    strcpy(ifr.ifr_name, port_name);
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        return 1;
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    return 0;
}

void can_bus_send(uint32_t id, uint8_t * data, uint8_t len)
{
    // printf("  CAN: -> ");
    // dp_h32(id);
    // printf(" [%d] ", len);
    // dp_hb8(buf, len);
    // write_console('\n');

    struct can_frame f;
    f.can_id = id + CAN_EFF_FLAG;
    f.can_dlc = len;
    for (size_t n = 0; n < len; n++) {
        f.data[n] = data[n];
    }
    write(s, &f, sizeof(struct can_frame));
}

uint8_t can_bus_recieve(uint32_t * id, uint8_t * data)
{
    struct can_frame f;
    int nbytes = read(s, &f, sizeof(struct can_frame));
    if (nbytes == -1) {
        return 0;
    }
    if (nbytes < sizeof(struct can_frame)) {
        return 0;
    }
    *id = f.can_id & CAN_EFF_MASK;
    for (size_t n = 0; n < f.can_dlc; n++) {
        data[n] = f.data[n];
    }
    // printf("<- frame(%d): ", nbytes);
    // print_frame(&f);
    // putchar('\n');
    return f.can_dlc;
}
