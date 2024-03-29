#include "bus77_en_ch.h"
#include "can_bus.h"
#include "debug_print.h"
#include "crc16.h"
#include <sys/time.h>

#define FRAME_TYPE_MODBUS_GATEWAY			0x5111
#define FRAME_TYPE_CHANNEL_CLOSE 			0x5210
#define FRAME_TYPE_CHANNEL_OPEN 			0x5011

typedef struct __attribute__((__packed__)) {
	uint16_t marker;
	uint8_t frame_len;
	uint16_t addr;
	uint16_t type;
	uint16_t n_msg;
	uint8_t n_flow;
	uint8_t n_block;
	uint16_t data_len;
	// data
	// crc16
} B77Header;

#define B77_HEADER_SIZE	(sizeof(B77Header))

static uint8_t b77_buf[256 + B77_HEADER_SIZE + 2 + 10];
static uint16_t b77_frame_msg_n = 0;
static uint16_t b77_frame_block_n = 0;
static uint8_t b77_frame_flow = 100;
static uint16_t b77_frame_addr = 0x0200;

static int b77_debug_level = 0;


void bus77_set_debug_level(int level)
{
	b77_debug_level = level;
}

static void bus77_send_frame(uint32_t id, uint8_t * data, uint16_t len)
{
	if (b77_debug_level) {
		printf(" bus77 ->: ");
		dp_hb8(data, len);
		putchar('\n');
	}

	uint16_t index = 0;
	while (index < (len - 8)) {
		can_bus_send(id, data + index, 8);
		index += 8;
		usleep(5 * 1000);
	}
	can_bus_send(id | 1, data + index, len - index);
}

static uint16_t bus77_recieve_frame(uint32_t * id, uint8_t * data)
{
    u8 len = 0;
	u8 index = 0;
    while(1) {
        len = can_bus_recieve(id, data + index);
        if (len == 0) {
            return index;	// frame len % 8 == 0, but last id lsb not 1
        }
        index += len;
        if (*id & 1) {
			*id--;
            // printf("<- iridium: ");
            // print_hex32(addr);
            // putchar(' ');
            // print_buf(rx_buf, ptr - rx_buf);
            // putchar('\n');
            return index;
        }
    }
}

static uint16_t bus77_prepare_frame(uint16_t type, uint8_t * data, uint16_t len)
{
	B77Header * header = b77_buf;
	header->marker = 0x087D;
	header->addr = b77_frame_addr;
	header->n_flow = b77_frame_flow;
	header->n_block = b77_frame_block_n++;
	header->n_msg = b77_frame_msg_n++;
	header->type = type;
	header->data_len = len;

	uint8_t msg_len = B77_HEADER_SIZE + len;
	header->frame_len = msg_len - 3;		// maker + frame_len

	for (uint16_t i = 0; i < len; i++) {
		b77_buf[B77_HEADER_SIZE + i] = data[i];
	}

	uint16_t crc = crc16_modbus(b77_buf + 5, msg_len - 5);

	b77_buf[msg_len + 0] = crc;
	b77_buf[msg_len + 1] = crc >> 8;

	return msg_len + 2;
}

void bus77_open_channel(void)
{
	if (b77_debug_level) {
		printf("[open bus77-modbus channel]\n");
	}
	uint8_t buffer[] = {0x7D, 0x08, 0x0D, 0x00, 0x01, 0x11, 0x50, 0x01, 0x00, 0x72, 0x73, 0x34, 0x38, 0x35, 0x00, 0x02, 0x2C, 0x66};
	bus77_send_frame(0x19998802, buffer, sizeof(buffer));
}

void bus77_close_channel(void)
{
	if (b77_debug_level) {
		printf("[close bus77-modbus channel]\n");
	}
	uint8_t buffer[] = {0x7D, 0x08, 0x07, 0x00, 0x01, 0x10, 0x52, 0x05, 0x00, 0x64, 0xE4, 0x91};
	bus77_send_frame(0x19999802, buffer, sizeof(buffer));
}

void bus77_send_modbus_frame(uint8_t * data, uint16_t len)
{
	uint16_t msg_len = bus77_prepare_frame(FRAME_TYPE_MODBUS_GATEWAY, data, len);
	bus77_send_frame(0x19999002, b77_buf, msg_len);
}


static void bus77_insert_crc(uint8_t * buf, uint16_t len)
{
	uint16_t crc = crc16_modbus(buf + 5, len - 5);
	buf[len + 0] = crc;
	buf[len + 1] = crc >> 8;
}

void bus77_set_modbus_baud(int baud)
{
	if (b77_debug_level) {
		printf("[change modbus bridge port baud to %d]\n", baud);
	}

	uint8_t buf1[] = {0x7D, 0x08, 0x0D, 0x00, 0x01, 0x11, 0x33, 0x46, 0x00, 0x02, 0x00, 0x00, 0x20, 0x85, 0x80, 0x04, 0xE5, 0xA1};
	switch (baud) {
	case 9600:
		buf1[15] = 0;
		buf1[14] = 96;
		break;

	default:
		break;
	}
	bus77_insert_crc(buf1, 16);
	bus77_send_frame(0x19999002, buf1, sizeof(buf1));

	if (b77_debug_level) {
		printf("[confirm change modbus bridge port baud]\n");
	}
	uint8_t buf2[] = {0x7D, 0x08, 0x0B, 0x00, 0x01, 0x11, 0x33, 0x49, 0x00, 0x05, 0x00, 0x00, 0xC0, 0x81, 0x0D, 0x05};
	bus77_send_frame(0x19999002, buf2, sizeof(buf2));
}

uint16_t bus77_recieve_modbus_frame(uint8_t * data)
{
	uint32_t id = 0;
	uint16_t len = 0;
	uint16_t timeout = 200;		// 2s

	while (1) {
		len = bus77_recieve_frame(&id, b77_buf);
		if (len != 0) {
			B77Header * header = b77_buf;
			if (b77_debug_level) {
				printf(" bus77 <-: ");
				dp_hb8(b77_buf, len);
				putchar('\n');
			}

			if (header->type == FRAME_TYPE_MODBUS_GATEWAY) {
				for (size_t i = 0; i < header->data_len; i++) {
					data[i] = b77_buf[B77_HEADER_SIZE + i];
				}
				return header->data_len;
			}
		} else {
			if (timeout) {
				timeout--;
			} else {
				return 0;
			}
		}
	}
}
