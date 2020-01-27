#include "modbus.h"
#include "nmisc.h"

#define MODBUS_CMD_READ_COILS					0x01
#define MODBUS_CMD_READ_DISCRETE_INPUTS			0x02
#define MODBUS_CMD_READ_HOLDING_REGISTERS		0x03
#define MODBUS_CMD_READ_INPUT_REGISTERS			0x04

#define MODBUS_CMD_WRITE_SINGLE_COIL			0x05
#define MODBUS_CMD_WRITE_SIGLE_REGISTER			0x06
#define MODBUS_CMD_WRITE_MULTIPLE_COIL			0x0F
#define MODBUS_CMD_WRITE_MULTIPLE_REGISTER		0x10


uint8_t modbus_buf[256];
uint16_t read_regs_count;

uint8_t modbus_write_single_coil(uint8_t id, uint16_t addr, uint16_t val)
{
	if (val) {
		val = 0xFF00;
	}
	modbus_buf[0] = id;
	modbus_buf[1] = MODBUS_CMD_WRITE_SINGLE_COIL;
	modbus_buf[2] = addr >> 8;
	modbus_buf[3] = addr;
	modbus_buf[4] = val >> 8;
	modbus_buf[5] = val;
	uint8_t len = 6;
	uint16_t crc = crc16_modbus(modbus_buf, len);
	modbus_buf[len++] = crc;
	modbus_buf[len++] = crc >> 8;
	return len;
}

uint8_t modbus_write_multiple_coil(uint8_t id, uint16_t addr, uint16_t * buf, uint8_t regs_amount)
{
	modbus_buf[0] = id;
	modbus_buf[1] = MODBUS_CMD_WRITE_MULTIPLE_COIL;
	modbus_buf[2] = addr >> 8;
	modbus_buf[3] = addr;
	modbus_buf[4] = regs_amount >> 8;
	modbus_buf[5] = regs_amount;
	modbus_buf[6] = regs_amount * 2;

	uint8_t len = 7;

	for (uint8_t i = 0; i < regs_amount; i++) {
		if ((i % 8) == 0) {
			len++;
			modbus_buf[len + (i / 8)] = 0;
		}
		u8 mask = 1 << (i % 8);
		if (buf[i]) {
			modbus_buf[len + (i / 8)] |= mask;
		}
	}
	len++;

	uint16_t crc = crc16_modbus(modbus_buf, len);
	modbus_buf[len++] = crc;
	modbus_buf[len++] = crc >> 8;
	return len;
}

uint8_t modbus_write_single_reg(uint8_t id, uint16_t addr, uint16_t val)
{
	modbus_buf[0] = id;
	modbus_buf[1] = MODBUS_CMD_WRITE_SIGLE_REGISTER;
	modbus_buf[2] = addr >> 8;
	modbus_buf[3] = addr;
	modbus_buf[4] = val >> 8;
	modbus_buf[5] = val;
	uint8_t len = 6;
	uint16_t crc = crc16_modbus(modbus_buf, len);
	modbus_buf[len++] = crc;
	modbus_buf[len++] = crc >> 8;
	return len;
}

uint8_t modbus_write_multiple_regs(uint8_t id, uint16_t addr, uint16_t * buf, uint8_t regs_amount)
{
	modbus_buf[0] = id;
	modbus_buf[1] = MODBUS_CMD_WRITE_MULTIPLE_REGISTER;
	modbus_buf[2] = addr >> 8;
	modbus_buf[3] = addr;
	modbus_buf[4] = regs_amount >> 8;
	modbus_buf[5] = regs_amount;
	modbus_buf[6] = regs_amount * 2;
	
	uint8_t len = 7;
	for (uint8_t i = 0; i < regs_amount; i++) {
		modbus_buf[len++] = buf[i] >> 8;
		modbus_buf[len++] = buf[i];
	}

	uint16_t crc = crc16_modbus(modbus_buf, len);
	modbus_buf[len++] = crc;
	modbus_buf[len++] = crc >> 8;
	return len;
}

uint8_t modbus_read_regs(uint8_t id, uint8_t cmd, uint16_t addr, uint16_t n_regs)
{
	read_regs_count = n_regs;
	modbus_buf[0] = id;
	modbus_buf[1] = cmd;
	modbus_buf[2] = addr >> 8;
	modbus_buf[3] = addr;
	modbus_buf[4] = n_regs >> 8;
	modbus_buf[5] = n_regs;
	uint8_t len = 6;
	uint16_t crc = crc16_modbus(modbus_buf, len);
	modbus_buf[len++] = crc;
	modbus_buf[len++] = crc >> 8;
	return len;
}

static uint16_t modbus_get_word(uint16_t index)
{
	uint16_t val = modbus_buf[index] << 8;
	val += modbus_buf[index + 1];
	return val;
}

uint8_t modbus_check_crc(uint16_t len)
{
	uint16_t crc = crc16_modbus(modbus_buf, len - 2);
	uint16_t frame_crc = modbus_buf[len - 1] << 8;
	frame_crc += modbus_buf[len - 2];
	if (frame_crc == crc) {
		return 1;
	}
	return 0;
}

uint8_t modbus_parse_answer(uint16_t len)
{
	if (modbus_check_crc(len) == 0) {
		printf("modbus wrong crc\n");
		return 0;
	}

	if ((modbus_buf[1] & 0x80)) {
		printf("modbus error: %d\n", modbus_buf[2]);
		return 0;
	}

	uint16_t val;
	printf("SUCCESS: ");
	switch (modbus_buf[1]) {
	case MODBUS_CMD_WRITE_SINGLE_COIL:
	case MODBUS_CMD_WRITE_SIGLE_REGISTER:
	case MODBUS_CMD_WRITE_MULTIPLE_COIL:
	case MODBUS_CMD_WRITE_MULTIPLE_REGISTER:
		val = *(uint16_t*)(&modbus_buf[4]);
		printf("written %d elements!\n", val);
		break;
	case MODBUS_CMD_READ_COILS:
	case MODBUS_CMD_READ_DISCRETE_INPUTS:
		printf("read elements: %d\n", read_regs_count);
		printf("\tData: ");
		for (size_t i = 0; i < read_regs_count; i++) {
			if ((modbus_buf[3  + (i / 8)]) & (1 << (i % 8))) {
				printf("1 ");
			} else {
				printf("0 ");
			}
		}
		putchar('\n');
		break;
	case MODBUS_CMD_READ_HOLDING_REGISTERS:
	case MODBUS_CMD_READ_INPUT_REGISTERS:
		printf("read elements:\n", *(uint16_t*)(&modbus_buf[4]));
		printf("\tData: ", *(uint16_t*)(&modbus_buf[4]));
		for (size_t i = 0; i < modbus_buf[2] / 2; i++) {
			val = modbus_get_word(3 + (2 * i));
			printf("0x");
			dp_h16(val);
			putchar(' ');
		}
		putchar('\n');
		break;
	default:
		break;
	}
	return 1;
}
