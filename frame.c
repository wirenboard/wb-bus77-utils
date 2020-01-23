#include "nmisc.h"

#include "can_bus.h"
#include "crc16.h"
#include "modbus.h"
#include "bus77_en_ch.h"

uint8_t a = 10;

typedef struct {
	uint8_t id;
	uint8_t cmd;
	uint16_t addr;
	uint16_t len;
	uint16_t crc;

} ModbusTxFrame;

uint8_t tbuf[] = {"ololo privet"};


uint16_t reg_to_write[] = {5, 0x14, 60, 381, 1};

int main(void)
{
	printf("Hey bitch!\r\n");

	can_bus_init("can0");
	// printf("Data to write: ");
	// dp_hb16(reg_to_write, 5);

	// u8 len = modbus_write_multiple_regs(41, 0x61, reg_to_write, 2);
	u8 len = modbus_read_regs(41, 0x80, 1);
	printf("\r\n");

	dp_hb8(modbus_buf, len);
	printf("\r\n");

	bus77_send(modbus_buf, len);

	return 0;
}
