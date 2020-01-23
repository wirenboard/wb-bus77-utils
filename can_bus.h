#include <stdint.h>

void can_bus_init(char * port_name);
void can_bus_send(uint32_t id, uint8_t * data, uint8_t len);
uint8_t can_bus_recieve(uint32_t * id, uint8_t * data);
