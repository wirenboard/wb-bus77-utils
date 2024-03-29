#include <stdint.h>

#ifndef MODBUS_H
#define MODBUS_H

#define MODBUS_CMD_READ_COILS					0x01
#define MODBUS_CMD_READ_DISCRETE_INPUTS			0x02
#define MODBUS_CMD_READ_HOLDING_REGISTERS		0x03
#define MODBUS_CMD_READ_INPUT_REGISTERS			0x04

#define MODBUS_CMD_WRITE_SINGLE_COIL			0x05
#define MODBUS_CMD_WRITE_SINGLE_REGISTER		0x06

#define MODBUS_CMD_WRITE_MULTIPLE_COIL			0x0F
#define MODBUS_CMD_WRITE_MULTIPLE_REGISTER		0x10

uint8_t modbus_make_frame(uint8_t id, uint8_t cmd, uint16_t addr, uint16_t * val_buf, uint16_t regs_amount, uint8_t * modbus_buf);
uint8_t modbus_parse_answer(uint8_t * modbus_buf, uint16_t len);

uint8_t modbus_is_error(uint8_t * modbus_buf);
uint8_t modbus_check_crc(uint8_t * modbus_buf, uint16_t len);

#endif
