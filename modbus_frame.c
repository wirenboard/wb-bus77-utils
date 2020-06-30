#include "modbus.h"
#include "crc16.h"
#include "debug_print.h"

uint8_t modbus_buf[256];
uint16_t regs_count;

static void modbus_put_word(uint16_t val, uint8_t * modbus_buf, uint16_t index)
{
	modbus_buf[index + 0] = val >> 8;
	modbus_buf[index + 1] = val;
}

static uint16_t modbus_get_word(uint8_t * modbus_buf, uint16_t index)
{
	uint16_t val = modbus_buf[index] << 8;
	val += modbus_buf[index + 1];
	return val;
}

uint8_t modbus_make_frame(uint8_t id, uint8_t cmd, uint16_t addr, uint16_t * val_buf, uint16_t regs_amount, uint8_t * modbus_buf)
{
	uint8_t len, bytes, i;
	modbus_buf[0] = id;
	modbus_buf[1] = cmd;
	modbus_put_word(addr, modbus_buf, 2);

	switch (cmd) {
	case MODBUS_CMD_READ_COILS:
	case MODBUS_CMD_READ_DISCRETE_INPUTS:
	case MODBUS_CMD_READ_HOLDING_REGISTERS:
	case MODBUS_CMD_READ_INPUT_REGISTERS:
		regs_count = regs_amount;
		modbus_put_word(regs_amount, modbus_buf, 4);
		len = 6;
		break;

	case MODBUS_CMD_WRITE_SINGLE_COIL:
		regs_count = 1;
		if (val_buf[0]) {
			modbus_buf[4] = 0xFF;
		} else {
			modbus_buf[4] = 0;
		}
		modbus_buf[5] = 0;
		len = 6;
		break;

	case MODBUS_CMD_WRITE_SINGLE_REGISTER:
		regs_count = 1;
		modbus_put_word(val_buf[0], modbus_buf, 4);
		len = 6;
		break;

	case MODBUS_CMD_WRITE_MULTIPLE_COIL:
		regs_count = regs_amount;
		modbus_put_word(regs_amount, modbus_buf, 4);
		bytes = 0;
		for (i = 0; i < regs_amount; i++) {
			if ((i % 8) == 0) {
				bytes++;
				modbus_buf[7 + (i / 8)] = 0;
			}
			u8 mask = 1 << (i % 8);
			if (val_buf[i]) {
				modbus_buf[7 + (i / 8)] |= mask;
			}
		}
		modbus_buf[6] = bytes;
		len = 7 + bytes;
		break;

	case MODBUS_CMD_WRITE_MULTIPLE_REGISTER:
		regs_count = regs_amount;
		modbus_put_word(regs_amount, modbus_buf, 4);
		modbus_buf[6] = regs_amount * 2;
		len = 7;
		for (i = 0; i < regs_amount; i++) {
			modbus_buf[len++] = val_buf[i] >> 8;
			modbus_buf[len++] = val_buf[i];
		}
		break;

	default:
		return 0;
		break;
	}

	uint16_t crc = crc16_modbus(modbus_buf, len);
	modbus_buf[len++] = crc;
	modbus_buf[len++] = crc >> 8;
	return len;
}

uint8_t modbus_is_error(uint8_t * modbus_buf)
{
	if ((modbus_buf[1] & 0x80)) {
		return 1;
	}
	return 0;
}

uint8_t modbus_check_crc(uint8_t * modbus_buf, uint16_t len)
{
	uint16_t crc = crc16_modbus(modbus_buf, len - 2);
	uint16_t frame_crc = modbus_buf[len - 1] << 8;
	frame_crc += modbus_buf[len - 2];
	if (frame_crc == crc) {
		return 1;
	}
	return 0;
}

uint8_t modbus_parse_answer(uint8_t * modbus_buf, uint16_t len)
{
	if (modbus_check_crc(modbus_buf, len) == 0) {
		printf("modbus wrong crc\n");
		return 0;
	}

	if (modbus_is_error(modbus_buf)) {
		printf("modbus error: %d\n", modbus_buf[2]);
		return 0;
	}

	uint16_t val;
	printf("SUCCESS: ");
	switch (modbus_buf[1]) {
	case MODBUS_CMD_WRITE_SINGLE_COIL:
	case MODBUS_CMD_WRITE_SINGLE_REGISTER:
	case MODBUS_CMD_WRITE_MULTIPLE_COIL:
	case MODBUS_CMD_WRITE_MULTIPLE_REGISTER:
		printf("written %d elements!\n", regs_count);
		break;
	case MODBUS_CMD_READ_COILS:
	case MODBUS_CMD_READ_DISCRETE_INPUTS:
		printf("read elements: %d\n", regs_count);
		printf("\tData: ");
		for (size_t i = 0; i < regs_count; i++) {
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
		printf("read elements: %d\n", regs_count);
		printf("\tData: ");
		for (size_t i = 0; i < modbus_buf[2] / 2; i++) {
			val = modbus_get_word(modbus_buf, 3 + (2 * i));
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
