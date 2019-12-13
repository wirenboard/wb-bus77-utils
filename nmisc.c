#include "nmisc.h"

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
