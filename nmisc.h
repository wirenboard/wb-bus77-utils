#ifndef NMISC_H
#define NMISC_H


#include <stdint.h>
#include <stdio.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;

void print_hex(u8 *ptr, u8 len, void (*write)(char c));
void print_hb(u8 *ptr, u8 grp, u8 len, u8 inl, void (*write)(char c));
void write_console(char c);

#define dbc							write_console
#define dp_hb8(buf, len)			print_hb(buf, 1, len, 0, dbc)
#define dp_hb16(buf, len)			print_hb(buf, 2, len, 0, dbc)
#define dp_h8(a) 					print_hex(&a, 1, dbc)
#define dp_h16(a) 					print_hex(&a, 2, dbc)
#define dp_h32(a) 					print_hex(&a, 4, dbc)

#endif