#include <stdint.h>
#include <stdio.h>

#include "crc16.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;


uint8_t a = 10;

typedef struct {
	uint8_t id;
	uint8_t cmd;
	uint16_t addr;
	uint16_t len;
	uint16_t crc;

} ModbusTxFrame;

uint8_t modbus_buf[256];

uint8_t tbuf[] = {"ololo privet"};

#define MODBUS_CMD_WRITE_SIGLE_REGISTER			0x06
#define MODBUS_CMD_WRITE_MULTIPLE_REGISTER		0x10


static const char hexvalues[] = "0123456789ABCDEF";

void print_hex(u8 *ptr, u8 len, void (*write)(char c))
{
	while (len--) {
		write(hexvalues[(*(ptr + len) >> 4) & 0x0F]);
		write(hexvalues[*(ptr + len) & 0x0F]);
	}
}

void print_hb(u8 *ptr, u8 grp, u8 len, u8 inl, void (*write)(char c))
{
	for (u8 i = 0; i < len; i++) {
		print_hex(ptr + grp * i, grp, write);
		if ((inl != 0) && ((i % inl) == (inl - 1))) {
			write('\r');
			write('\n');
		} else {
			write(' ');
		}
	}
}

void write_console(char c)
{
	putchar(c);
}


#define dbc							write_console
#define dp_hb8(buf, instr, len)		print_hb(buf, 1, len, instr, dbc)
#define dp_h8(a) 					print_hex(&a, 1, dbc)
#define dp_h16(a) 					print_hex(&a, 2, dbc)
#define dp_h32(a) 					print_hex(&a, 4, dbc)


void modbus_write_single_reg(uint8_t id, uint16_t addr, uint16_t val)
{
	modbus_buf[0] = id;
	modbus_buf[1] = MODBUS_CMD_WRITE_SIGLE_REGISTER;
	modbus_buf[2] = addr >> 8;
	modbus_buf[3] = addr;
	modbus_buf[4] = val >> 8;
	modbus_buf[5] = val;
	uint16_t crc = crc16_modbus(modbus_buf, 6);
	modbus_buf[6] = crc;
	modbus_buf[7] = crc >> 8;
}




int main(void)
{
	printf("Hey bitch!\r\n");

	modbus_write_single_reg(10, 128, 40);

	printf("\r\n");
	dp_hb8(modbus_buf, 0, 8);

	printf("\r\n");
	return 0;
}
