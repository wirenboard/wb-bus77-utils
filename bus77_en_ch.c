#include "bus77_en_ch.h"
#include "can_bus.h"
#include "nmisc.h"
#include "crc16.h"

typedef struct __attribute__((__packed__)) {
	uint16_t marker;
	uint8_t len;
	uint16_t addr;
	uint16_t header;
	uint16_t n_msg;
	uint8_t n_flow;
	uint8_t n_block;
	uint16_t data_len;
} B77Header;

uint8_t b77_buf[256 + sizeof(B77Header) + 2];
uint16_t n_msg;


void bus77_send(uint8_t * buf, uint8_t len)
{
	B77Header * header = b77_buf;
	header->addr = 0x0100;
	header->marker = 0x087D;
	header->n_flow = 100;
	header->header = 0x5111;
	header->n_block = 1;
	header->n_msg = 3;
	header->data_len = len;


	uint8_t msg_len = sizeof(B77Header) + len; 
	header->len = msg_len - 3;

	for (uint16_t i = 0; i < len; i++) {
		b77_buf[sizeof(B77Header) + i] = buf[i];
	}
	
	uint16_t crc = crc16_modbus(b77_buf + 5, msg_len - 5);

	b77_buf[msg_len + 0] = crc;
	b77_buf[msg_len + 1] = crc >> 8;

	msg_len += 2;

	uint8_t index = 0;
	while (index < (msg_len - 8)) {
		can_bus_send(0x19999002, &(b77_buf[index]), 8);
		index += 8;
	}
	can_bus_send(0x19999003, &(b77_buf[index]), msg_len - index);

	// printf("BUS 77 Frame: ");
	// dp_hb8(b77_buf, sizeof(B77Header) + len + 2);
	// printf("\r\n");
}
