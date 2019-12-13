#include <stdint.h>

#ifndef MODBUS_H
#define MODBUS_H

typedef enum  {
	WAIT,
	OK,
	ERROR
} ModbusStatus;

uint8_t modbus_write_multiple_regs(uint8_t id, uint16_t addr, uint16_t * buf, uint8_t regs_amount);
uint8_t modbus_write_single_reg(uint8_t id, uint16_t addr, uint16_t val);
uint8_t modbus_read_regs(uint8_t id, uint16_t addr, uint16_t n_regs);

extern uint8_t modbus_buf[];

#endif
