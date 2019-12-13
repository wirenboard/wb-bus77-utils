#include "modbus.h"

#define MODBUS_CMD_READ_REGISTERS				0x03
#define MODBUS_CMD_WRITE_SIGLE_REGISTER			0x06
#define MODBUS_CMD_WRITE_MULTIPLE_REGISTER		0x10

uint8_t modbus_buf[256];
ModbusStatus modbus_status;
uint8_t modbus_error;
/*
нужно получать ответы, асинхронщина таймер

*/
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

	modbus_error = 0;


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

uint8_t modbus_read_regs(uint8_t id, uint16_t addr, uint16_t n_regs)
{
	modbus_buf[0] = id;
	modbus_buf[1] = MODBUS_CMD_READ_REGISTERS;
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
