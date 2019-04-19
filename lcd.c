#include <F28x_Project.h>
#include "OneToOneI2CDriver.h"

const Uint16 initCommands[] = {0x33, 0x32, 0x28, 0x0C, 0x01};
const Uint16 shiftCommand[] = {0xC0};
const Uint16 clearCommand[] = {0x01};

Uint16 strlen(const char string[]) {
    Uint16 i = 0;
    while(string[i] != 0) {
        i++;
    }
    return i;
}

void write_commands(const Uint16 commands[], Uint16 len) {
    for(Uint16 i = 0; i < len; i++) {
        Uint16 values[4];
        values[0] = 0x000C | (0x00F0 & commands[i]);
        values[1] = 0x0008 | (0x00F0 & commands[i]);
        values[2] = 0x000C | (0x00F0 & commands[i]<<4);
        values[3] = 0x0008 | (0x00F0 & commands[i]<<4);
        I2C_O2O_SendBytes(values, 4);
        DELAY_US(1E3);
    }
}

void write_string(const char string[]) {
    Uint16 len = strlen(string);
    for(Uint16 i = 0; i < len; i++) {
        Uint16 values[4];
        values[0] = 0x000D | (0x00F0 & string[i]);
        values[1] = 0x0009 | (0x00F0 & string[i]);
        values[2] = 0x000D | (0x00F0 & string[i]<<4);
        values[3] = 0x0009 | (0x00F0 & string[i]<<4);
        I2C_O2O_SendBytes(values, 4);
        DELAY_US(1E3);
    }
}

void setup_lcd() {
    I2C_O2O_Master_Init(0x003F, 200, 100);
    write_commands(initCommands, 5); //init
    DELAY_US(1E3);
}
