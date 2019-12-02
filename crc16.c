#include "crc16.h"

uint16_t crc16_modbus(uint8_t * buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < len; pos++) {
        crc ^= buf[pos];                        // XOR byte into least sig. byte of crc

        for (uint8_t i = 8; i != 0; i--) {      // Loop over each bit
            if ((crc & 0x0001) != 0) {          // If the LSB is set
                crc >>= 1;                      // Shift right and XOR 0xA001
                crc ^= 0xA001;
            } else {                            // Else LSB is not set
                crc >>= 1;                      // Just shift right
            }
        }
    }
    // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
    return crc;
}
