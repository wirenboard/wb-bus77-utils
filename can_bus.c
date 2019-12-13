#include "can_bus.h"
#include "nmisc.h"

void can_bus_send(uint32_t id, uint8_t * buf, uint8_t len)
{
	printf("  CAN: -> ");
	dp_h32(id);
	printf(" [%d] ", len);
	dp_hb8(buf, len);
	write_console('\n');
}