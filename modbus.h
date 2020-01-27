#include <stdint.h>

#ifndef MODBUS_H
#define MODBUS_H

#define MODBUS_CMD_READ_COILS					0x01
#define MODBUS_CMD_READ_DISCRETE_INPUTS			0x02
#define MODBUS_CMD_READ_HOLDING_REGISTERS		0x03
#define MODBUS_CMD_READ_INPUT_REGISTERS			0x04

#define MODBUS_CMD_WRITE_SINGLE_COIL			0x05
#define MODBUS_CMD_WRITE_SIGLE_REGISTER			0x06

#define MODBUS_CMD_WRITE_MULTIPLE_COIL			0x0F
#define MODBUS_CMD_WRITE_MULTIPLE_REGISTER		0x10

uint8_t modbus_read_regs(uint8_t id, uint8_t cmd, uint16_t addr, uint16_t n_regs);
uint8_t modbus_write_single_reg(uint8_t id, uint16_t addr, uint16_t val);
uint8_t modbus_write_multiple_regs(uint8_t id, uint16_t addr, uint16_t * buf, uint8_t regs_amount);
uint8_t modbus_write_single_coil(uint8_t id, uint16_t addr, uint16_t val);
uint8_t modbus_write_multiple_coil(uint8_t id, uint16_t addr, uint16_t * buf, uint8_t regs_amount);

uint8_t modbus_parse_answer(uint16_t len);

extern uint8_t modbus_buf[];

#endif
