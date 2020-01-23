#include <stdint.h>

void bus77_send_modbus_frame(uint8_t * data, uint16_t len);
uint16_t bus77_recieve_modbus_frame(uint8_t * data);
void bus77_open_channel(void);
void bus77_close_channel(void);
