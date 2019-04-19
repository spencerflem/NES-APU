#include <F28x_Project.h>
#include <math.h>
#include <stdbool.h>
#include "adc.h"
#include "interrupt.h"
#include "lcd.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "APU.h"

interrupt void mcBspRx();


char file[] = {0xB4, 0x17, 0xC0,
               0xB4, 0x15, 0x0F,
                   0x73,
               0xB4, 0x06, 0x8E,
               0xB4, 0x07, 0x08,
               0xB4, 0x05, 0x7F,
               0xB4, 0x04, 0x86,
                   0x61, 0xA8, 0x1C,
               0xB4, 0x06, 0x97,
                   0x70,
               0xB4, 0x07, 0x08,
                   0x61, 0xA9, 0x1C,
               0xB4, 0x06, 0xB3,
                   0x70,
               0xB4, 0x07, 0x08,
                   0x61, 0xA9, 0x1C,
               0xB4, 0x06, 0xFE,
                   0x70,
               0xB4, 0x07, 0x08,
                   0x61, 0xA9, 0x1C,
               0xB4, 0x06, 0x0D,
                   0x70,
               0xB4, 0x07, 0x09,
                   0x61, 0xA9, 0x1C,
               0xB4, 0x06, 0xA9,
                   0x70,
               0xB4, 0x07, 0x08,
                   0x61, 0xA9, 0x1C,
               0xB4, 0x06, 0x86,
                   0x70,
               0xB4, 0x07, 0x08,
                   0x61, 0xAA, 0x1C,
               0xB4, 0x06, 0x6A,
               0xB4, 0x07, 0x08,
                   0x61, 0xD7, 0xD9,
               0xB4, 0x04, 0x90,
               0xB4, 0x07, 0x18,
                   0x70,
               0xB4, 0x06, 0x00,
                   0x61, 0x7F, 0x05,
                   0x66};

volatile bool even;
volatile float cyclesToProcess = 0;
int32 index = 0;
Uint32 wait = 0;
int32 sample = 0;

int main(void) {

    // inits
    InitSysCtrl();
    InitBigBangedCodecSPI();
    InitAIC23(SR48);
    InitMcBSPb();
    Interrupt_initModule();
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_enableMaster();

    struct Apu apu = {};
    initApu(&apu);

    while (1) {
        if (cyclesToProcess > 0) {
            if (wait == 0) {
                while (wait == 0) {
                    Uint16 command = file[index++];
                    if (command == 0xB4) {
                        Uint16 address = file[index++];
                        Uint16 data = file[index++];
                        writeRegister(&apu, address, data);
                    }
                    else if (command & 0xF0 == 0x70) {
                        wait = (command & 0x0F) + 1;
                    }
                    else if (command == 0x61) {
                        Uint16 waitLo = file[index++];
                        Uint16 waitHi = file[index++];
                        wait = waitHi << 8 | waitLo;
                    }
                    else if (command == 0x62) {
                        wait = 735;
                    }
                    else if (command == 0x66) {
                        // END OF FILE;
                        wait=1;
                        index--;
                    }
                 }
            }
            else {
                wait--;
            }
            sample = processCycles(&apu, cyclesToProcess);
            cyclesToProcess = 0;
        }
    }

}

interrupt void mcBspRx() {
    if(even) {
        even = false;
    }
    else {
        even = true;
        cyclesToProcess += cyclesPerSample;
    }
    // dummy read
    Uint16 lo = McbspbRegs.DRR1.all;
    Uint16 hi = McbspbRegs.DRR2.all;
    // output the current sample
    McbspbRegs.DXR1.all = sample & 0xFFFF;
    McbspbRegs.DXR2.all = (sample >> 16) & 0xFFFF;
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
