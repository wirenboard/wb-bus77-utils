#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "debug_print.h"

#include "modbus_frame.h"
#include "can_bus.h"
#include "bus77_en_ch.h"

#define INFO_BLOCK_SIZE             32
#define INFO_BLOCK_REG_ADDRESS      0x1000

#define DATA_BLOCK_SIZE             136
#define DATA_BLOCK_REG_ADDRESS      0x2000

#define MAX_ERROR_COUNT             3

#define HOLD_REG_JUMP_TO_BOOTLOADER         129
#define HOLD_REG_CMD_UART_SETTINGS_RESET    1000
#define HOLD_REG_CMD_EEPROM_ERASE           1001

#define xstr(a) str(a)
#define str(a) #a

const char flashingExample[] = "-d <port> -f <firmware.wbfw>";
const char formatSettingsExample[] = "-d <port> -j -u";
const char casualUsageExample[] = "-d <port> -a <modbus_addr> -j -u -f <firmware.wbfw>";

struct UartSettings {
    int baudrate;
    char parity;
    int databits;
    int stopbits;
} UartSettings;

const int allowedBaudrates[] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400};
const int allowedStopBits[] = {1, 2};
const char allowedParity[] = {'N', 'E', 'O'};

int ensureIntIn(int param, const int array[], unsigned int arrayLen);
int ensureCharIn(char param, const char array[], unsigned int arrayLen);

int mode_iridium_can = 0;
uint8_t iridium_mb_frame[280];

int iridium_can_modbus_write_register(uint8_t id, uint16_t addr, uint16_t val)
{
    int len = modbus_make_frame(id, MODBUS_CMD_WRITE_SINGLE_REGISTER, addr, &val, 1, iridium_mb_frame);
    bus77_send_modbus_frame(iridium_mb_frame, len);
    len = bus77_recieve_modbus_frame(iridium_mb_frame);
    if (modbus_check_crc(iridium_mb_frame, len) == 0) {
        return 0;
    }
    if (modbus_is_error(iridium_mb_frame)) {
        return 0;
    }
    return 1;
}

int iridium_can_modbus_write_registers(uint8_t id, uint16_t addr, uint16_t nreg, uint16_t * vals)
{
	int len = modbus_make_frame(id, MODBUS_CMD_WRITE_MULTIPLE_REGISTER, addr, vals, nreg, iridium_mb_frame);
	printf("modbus frame > : ");
    dp_hb8(iridium_mb_frame, len);
    putchar('\n');

	bus77_send_modbus_frame(iridium_mb_frame, len);
    len = bus77_recieve_modbus_frame(iridium_mb_frame);
	printf("modbus frame < : ");
    dp_hb8(iridium_mb_frame, len);
    putchar('\n');
    if (modbus_check_crc(iridium_mb_frame, len) == 0) {
        return 0;
    }
    if (modbus_is_error(iridium_mb_frame)) {
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc == 1) {
        printf("Welcome to Wiren Board flash tool.\n");
        printf("Version: " xstr(VERSION) " for bus 77 wirenboard devices\n\n");
        printf("Usage:\n\n");

        printf("Param  Description                                         Default value\n\n");
        printf("-d     Serial port (e.g. \"/dev/ttyRS485-1\")                      -\n");
        printf("-f     Firmware file                                             -\n");
        printf("-a     Modbus ID (slave addr)                                    1\n");
        printf("-j     Send jump to bootloader command                           -\n");
        printf("-u     Reset UART setting and MODBUS address to factory default  -\n");
        printf("-e     Reset ALL settings to factory default                     -\n");
        printf("-r     Jump to bootloader register address                       129\n");
        printf("-D     Debug mode                                                -\n");
        printf("-b     Baud Rate (serial port speed)                             9600\n");
        printf("-p     Parity                                                    N\n");
        printf("-s     Stopbits                                                  2\n");

        printf("\nMinimal flashing example:\n%s %s\n", argv[0], flashingExample);
        printf("Minimal format uart settings example:\n%s %s\n", argv[0], formatSettingsExample);
        printf("Flashing running device example:\n%s %s\n", argv[0], casualUsageExample);
        return 0;
    };

    struct UartSettings bootloaderParams = { //Bootloader has fast flash mode with high baudrate
        .baudrate = 9600,
        .parity = 'N',
        .databits = 8,
        .stopbits = 2
    };

    struct UartSettings deviceParams = { //To send -j to device. Filled from user input
        .baudrate = 9600,
        .parity = 'N',
        .databits = 8,
        .stopbits = 2
    };

    // Default values
    char *device   = NULL;
    char *fileName = NULL;
    int   modbusID = 1;
    int   jumpCmd  = 0;
    int   uartResetCmd = 0;
    int   eepromFormatCmd = 0;
    int   jumpReg  = HOLD_REG_JUMP_TO_BOOTLOADER;
    int   debug    = 0;
    int   inBootloader = 0;

    int c;
    while ((c = getopt(argc, argv, "d:f:a:juer:Db:p:s:B:")) != -1) {
        switch (c) {
        case 'd':
            device = optarg;
            break;
        case 'f':
            fileName = optarg;
            break;
        case 'a':
            sscanf(optarg, "%d", &modbusID);
            break;
        case 'j':
            jumpCmd = 1;
            break;
        case 'u':
            uartResetCmd = 1;
            break;
        case 'e':
            eepromFormatCmd = 1;
            break;
        case 'r':
            sscanf(optarg, "%d", &jumpReg);
            break;
        case 'D':
            debug = 1;
            break;
        case 'b':
            sscanf(optarg, "%d", &deviceParams.baudrate);
            if (ensureIntIn(deviceParams.baudrate, allowedBaudrates, sizeof(allowedBaudrates))) {
                break;
            } else {
                printf("Baudrate (-b <%d>) is not supported!\n", deviceParams.baudrate);
                exit(EXIT_FAILURE);
            };
        case 'B':
            sscanf(optarg, "%d", &bootloaderParams.baudrate);
            if (ensureIntIn(bootloaderParams.baudrate, allowedBaudrates, sizeof(allowedBaudrates))) {
                break;
            } else {
                printf("Baudrate (-B <%d>) is not supported!\n", bootloaderParams.baudrate);
                exit(EXIT_FAILURE);
            };
        case 'p':
            sscanf(optarg, "%c", &deviceParams.parity);
            if (ensureCharIn(deviceParams.parity, allowedParity, sizeof(allowedParity))) {
                break;
            } else {
                printf("Parity (-p <%c>) is not supported!\n", deviceParams.parity);
                exit(EXIT_FAILURE);
            };
        case 's':
            sscanf(optarg, "%d", &deviceParams.stopbits);
            if (ensureIntIn(deviceParams.stopbits, allowedStopBits, sizeof(allowedStopBits))) {
                break;
            } else {
                printf ("Stopbits (-s <%d>) are not supported!\n", deviceParams.stopbits);
                exit(EXIT_FAILURE);
            };
        default:
            printf("Parameters error.\n");
            break;
        }
    }


    if (device == NULL) {
        printf("A port should be specified!\n%s -d <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

	// if (strncmp(device, "can", 3) == 1) {
	// 	printf("This flasher only for bus 77 wirenboard devices");
	// 	return 1;
	// }

	can_bus_init(device);
	bus77_open_channel();

	if (jumpCmd) {
		printf("Send jump to bootloader command and wait 2 seconds...\n");
		if (iridium_can_modbus_write_register(modbusID, jumpReg, 1)) {
			printf("Ok, device will jump to bootloader.\n");
			inBootloader = 1;
		} else {
			fprintf(stderr, "Device probably doesn't support in-field firmware upgrade\n");
			exit(EXIT_FAILURE);
		}
		sleep(2);    // wait 2 seconds
	}

    if (uartResetCmd) {
        printf("Send reset UART settings and modbus address command...\n");
        if (iridium_can_modbus_write_register(modbusID, HOLD_REG_CMD_UART_SETTINGS_RESET, 1)) {
            printf("Ok.\n");
            inBootloader = 1;
		}
        sleep(1);    // wait 1 second
    }

    if (eepromFormatCmd) {
        printf("Send format EEPROM command...\n");
        if (iridium_can_modbus_write_register(modbusID, HOLD_REG_CMD_EEPROM_ERASE, 1)) {
            printf("Ok.\n");
            inBootloader = 1;
        }
        sleep(1);    // wait 1 second
    }

    if (fileName == NULL) {
        if (inBootloader) {
            printf ("Device is in Bootloader now! To flash FW run\n%s %s\n", argv[0], flashingExample);
        } else {
            printf("To flash FW on running device, run\n%s %s\n", argv[0], casualUsageExample);
        }
        return 0;
    }

    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error while opening firmware file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fseek(file, 0L, SEEK_END);
    unsigned int filesize = ftell(file);
    printf("%s opened successfully, size %d bytes\n", fileName, filesize);
    rewind(file);

    uint16_t *data = malloc(filesize);
    if (fread(data, 1, filesize, file) != filesize) {
        fprintf(stderr, "Error while reading firmware file: %s\n", strerror(errno));
    }

	printf("Copy to buffer...");
    for (unsigned int i = 0; i < filesize / 2; i++) {
        data[i] = ((data[i] & 0xFF) << 8) | ((data[i] & 0xFF00) >> 8);
    }

	printf("OK\n");

    int errorCount = 0;
    unsigned int filePointer = 0;

    printf("\nSending info block...\n");
    while (errorCount < MAX_ERROR_COUNT) {
        if (iridium_can_modbus_write_registers(modbusID, INFO_BLOCK_REG_ADDRESS, INFO_BLOCK_SIZE / 2, &data[filePointer / 2])) {
            printf(" OK\n");
            filePointer += INFO_BLOCK_SIZE;
           break;
       }
		return 1;
    }

    printf("\n");
    while (filePointer < filesize) {
        fflush(stdout);
        printf("\rSending data block %u of %u...",
               (filePointer - INFO_BLOCK_SIZE) / DATA_BLOCK_SIZE + 1,
               (filesize - INFO_BLOCK_SIZE) / DATA_BLOCK_SIZE); fflush(stdout);
        if (iridium_can_modbus_write_registers(modbusID, DATA_BLOCK_REG_ADDRESS, DATA_BLOCK_SIZE / 2, &data[filePointer / 2])) {
            filePointer += DATA_BLOCK_SIZE;
            errorCount = 0;
        } else {
			exit(EXIT_FAILURE);
        }
    }

    printf(" OK.\n\nAll done!\n");
   	bus77_close_channel();

    fclose(file);
    free(data);

    exit(EXIT_SUCCESS);
}

int ensureIntIn(int param, const int array[], unsigned int arrayLen) {
    int valueIsIn = 0;
    for (unsigned int i = 0; i < arrayLen; i++){
        if (param == array[i]){
            valueIsIn = 1;
            break;
        }
    }
    return valueIsIn;
}


int ensureCharIn(char param, const char array[], unsigned int arrayLen) {
    int valueIsIn = 0;
    for (unsigned int i = 0; i < arrayLen; i++){
        if (param == array[i]){
            valueIsIn = 1;
            break;
        }
    }
    return valueIsIn;
}
