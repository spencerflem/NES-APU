#include <F28x_Project.h>
#include <stdbool.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "APU.h"
#include "Gui.h"

interrupt void mcBspRx();

volatile bool even;
volatile float cyclesToProcess = 0;
int32 sample = 0;

int main(void) {

    // inits
    InitSysCtrl();
    initGuiGpio();
    InitBigBangedCodecSPI();
    InitAIC23(SR48);
    InitMcBSPb();
    Interrupt_initModule();
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_enableMaster();

    Gui gui;
    Apu apu;
    initApu(&apu);
    initGui(&gui, &apu);

    while (1) {

        updateGuiState(&gui);
        displayGuiState(&gui);

        if(gui.playFile) {
            if (cyclesToProcess > 0) {
                // PLAY FROM FILE MODE
                if (gui.wait == 0) {
                    while (gui.wait == 0) {
                        Uint16 command = gui.file[gui.index++];
                        if (command == 0xB4) {
                            Uint16 address = gui.file[gui.index++];
                            Uint16 data = gui.file[gui.index++];
                            writeRegister(&apu, address, data);
                        }
                        else if (command & 0xF0 == 0x70) {
                            gui.wait = (command & 0x0F) + 1;
                        }
                        else if (command == 0x61) {
                            Uint16 waitLo = gui.file[gui.index++];
                            Uint16 waitHi = gui.file[gui.index++];
                            gui.wait = waitHi << 8 | waitLo;
                        }
                        else if (command == 0x62) {
                            gui.wait = 735;
                        }
                        else if (command == 0x66) {
                            gui.index = gui.fileLoopIndex;
                        }
                     }
                }
                else {
                    gui.wait--;
                }
                sample = processCycles(&apu, cyclesToProcess);
                cyclesToProcess = 0;
            }
        }

        else if (gui.playLoop) {
            if (cyclesToProcess > 0) {
                // PLAY FROM LOOP MODE
                if (gui.wait == 0) {
                    while (gui.wait == 0) {
                        Uint16 command = gui.file[gui.index++];
                        if (command == 0xB4) {
                            Uint16 address = gui.file[gui.index++];
                            Uint16 data = gui.file[gui.index++];
                            writeRegister(&apu, address, data);
                        }
                        else if (command & 0xF0 == 0x70) {
                            gui.wait = (command & 0x0F) + 1;
                        }
                        else if (command == 0x61) {
                            Uint16 waitLo = gui.file[gui.index++];
                            Uint16 waitHi = gui.file[gui.index++];
                            gui.wait = waitHi << 8 | waitLo;
                        }
                        else if (command == 0x62) {
                            gui.wait = 735;
                        }
                        else if (command == 0x66) {
                            // END OF FILE;
                            initApu(&apu);
                            gui.wait=1;
                            gui.index--;
                        }
                     }
                }
                else {
                    gui.wait--;
                }
                sample = processCycles(&apu, cyclesToProcess);
                cyclesToProcess = 0;
            }

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
