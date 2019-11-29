#include <stdint.h>
#include <stdio.h>


uint8_t a = 10;


int main(void)
{
	printf("Hey bitch!\r\n");

	printf("Size of char: %ld\r\n", sizeof(char));
	printf("Size of int: %ld\r\n", sizeof(int));
	printf("Size of long: %ld\r\n", sizeof(long));
	printf("Size of uint8_t: %ld\r\n", sizeof(uint8_t));
	printf("Size of uint16_t: %ld\r\n", sizeof(uint16_t));
	printf("Size of uint32_t: %ld\r\n", sizeof(uint32_t));


	return 0;
}