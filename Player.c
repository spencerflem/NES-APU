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

//octaves, Lo to Hi, C..B
Uint16 periodTable[6][12] = {{0x06ad,0x064d,0x05f3,0x059d,0x054c,0x0500,0x04b8,0x0474,0x0434,0x03f8,0x03bf,0x0389},
                             {0x0356,0x0326,0x02f9,0x02ce,0x02a6,0x0280,0x025c,0x023a,0x021a,0x01fb,0x01df,0x01c4},
                             {0x01ab,0x0193,0x017c,0x0167,0x0152,0x013f,0x012d,0x011c,0x010c,0x00fd,0x00ef,0x00e1},
                             {0x00d5,0x00c9,0x00bd,0x00b3,0x00a9,0x009f,0x0096,0x008e,0x0086,0x007e,0x0077,0x0070},
                             {0x006a,0x0064,0x005e,0x0059,0x0054,0x004f,0x004b,0x0046,0x0042,0x003f,0x003b,0x0038},
                             {0x0034,0x0031,0x002f,0x002c,0x0029,0x0027,0x0025,0x0023,0x0021,0x001f,0x001d,0x001b}};

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
                            writeRegister(gui.apu, address, data);
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
                if (gui.wait == 0) {
                    while (gui.wait == 0) {
                        Uint16 command = gui.oldBuff[gui.index++];
                        if (command == 0xB4) {
                            Uint16 address = gui.oldBuff[gui.index++];
                            Uint16 data = gui.oldBuff[gui.index++];
                            writeRegister(gui.apu, address, data);
                        }
                        else if (command & 0xF0 == 0x70) {
                            gui.wait = (command & 0x0F) + 1;
                        }
                        else if (command == 0x61) {
                            Uint16 waitLo = gui.oldBuff[gui.index++];
                            Uint16 waitHi = gui.oldBuff[gui.index++];
                            gui.wait = waitHi << 8 | waitLo;
                        }
                        else if (command == 0x62) {
                            gui.wait = 735;
                        }
                        else if (command == 0x66) {
                            gui.index = 0;
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

        else if (gui.recordLoop > 0) {
            if (cyclesToProcess > 0) {
                if (gui.wait == 0) {
                    if (gui.waitTotal > 0) {
                        gui.newBuff[gui.newIndex++] = 0x61;
                        gui.newBuff[gui.newIndex++] = gui.waitTotal & 0xFF;
                        gui.newBuff[gui.newIndex++] = (gui.waitTotal << 8) & 0xFF;
                    }
                    while (gui.wait == 0) {
                        Uint16 command = gui.oldBuff[gui.index++];
                        if (command == 0xB4) {
                            Uint16 address = gui.oldBuff[gui.index++];
                            Uint16 data = gui.oldBuff[gui.index++];
                            if (!gui.recPlaying
                                    || !((gui.recordLoop == 1 && address <= 3)
                                            || (gui.recordLoop == 2 && address >= 4 && address <= 7)
                                            || (gui.recordLoop == 3 && address >= 8 && address <= 11)
                                            || (gui.recordLoop == 4 && address >= 12 && address <= 15))) {
                                //dont overwrite currently playing instrument
                                writeRegister(gui.apu, address, data);
                                gui.newBuff[gui.newIndex++] = command;
                                gui.newBuff[gui.newIndex++] = address;
                                gui.newBuff[gui.newIndex++] = data;
                            }
                        }
                        else if (command & 0xF0 == 0x70) {
                            gui.wait = (command & 0x0F) + 1;
                            gui.waitTotal = (command & 0x0F) + 1;
                        }
                        else if (command == 0x61) {
                            Uint16 waitLo = gui.oldBuff[gui.index++];
                            Uint16 waitHi = gui.oldBuff[gui.index++];
                            gui.wait = waitHi << 8 | waitLo;
                            gui.waitTotal = waitHi << 8 | waitLo;
                        }
                        else if (command == 0x62) {
                            gui.wait = 735;
                            gui.waitTotal = 735;
                        }
                        else if (command == 0x66) {
                            //end recording
                            switch(gui.recordLoop) {
                            case 0:
                                gotoState(&gui, guiRecPu1);
                                break;
                            case 1:
                                gotoState(&gui, guiRecPu2);
                                break;
                            case 2:
                                gotoState(&gui, guiRecTri);
                                break;
                            case 3:
                                gotoState(&gui, guiRecNoi);
                                break;
                            }
                            initApu(gui.apu);
                            gui.recordLoop = 0;
                        }
                     }
                }
                else {
                    gui.wait--;

                    int16 newKey = 0;
                    int16 loseKey = 0;

                    // Check if key just pressed
                    if (!GpioDataRegs.GPDDAT.bit.GPIO124 && !gui.press0) { newKey = 1; gui.press0 = 0xFFFF; }
                    else if (!GpioDataRegs.GPDDAT.bit.GPIO125 && !gui.press1) { newKey = 2; gui.press1 = 0xFFFF; }
                    else if (!GpioDataRegs.GPCDAT.bit.GPIO95 && !gui.press2) { newKey = 3; gui.press2 = 0xFFFF; }
                    else if (!GpioDataRegs.GPEDAT.bit.GPIO139 && !gui.press3) { newKey = 4; gui.press3 = 0xFFFF; }
                    else if (!GpioDataRegs.GPBDAT.bit.GPIO56 && !gui.press4) { newKey = 5; gui.press4 = 0xFFFF; }
                    else if (!GpioDataRegs.GPDDAT.bit.GPIO97 && !gui.press5) { newKey = 6; gui.press5 = 0xFFFF; }
                    else if (!GpioDataRegs.GPCDAT.bit.GPIO94 && !gui.press6) { newKey = 7; gui.press6 = 0xFFFF; }
                    else if (!GpioDataRegs.GPCDAT.bit.GPIO65 && !gui.press7) { newKey = 8; gui.press7 = 0xFFFF; }
                    else if (!GpioDataRegs.GPCDAT.bit.GPIO66 && !gui.press8) { newKey = 9; gui.press8 = 0xFFFF; }
                    else if (!GpioDataRegs.GPCDAT.bit.GPIO67 && !gui.press9) { newKey = 10; gui.press9 = 0xFFFF; }
                    else if (!GpioDataRegs.GPDDAT.bit.GPIO111 && !gui.press10) { newKey = 11; gui.press10 = 0xFFFF; }
                    else if (!GpioDataRegs.GPBDAT.bit.GPIO32 && !gui.press11) { newKey = 12; gui.press11 = 0xFFFF; }
                    else if (!GpioDataRegs.GPDDAT.bit.GPIO123 && !gui.press12) { newKey = 13; gui.press12 = 0xFFFF; }
                    else if (!GpioDataRegs.GPBDAT.bit.GPIO59 && !gui.press13) { newKey = 14; gui.press13 = 0xFFFF; }

                    // Check if key just released
                    if (GpioDataRegs.GPDDAT.bit.GPIO124 && gui.press0) { loseKey = 1; gui.press0--; }
                    else if (GpioDataRegs.GPDDAT.bit.GPIO125 && gui.press1) { loseKey = 2; gui.press1--; }
                    else if (GpioDataRegs.GPCDAT.bit.GPIO95 && gui.press2) { loseKey = 3; gui.press2--; }
                    else if (GpioDataRegs.GPEDAT.bit.GPIO139 && gui.press3) { loseKey = 4; gui.press3--; }
                    else if (GpioDataRegs.GPBDAT.bit.GPIO56 && gui.press4) { loseKey = 5; gui.press4--; }
                    else if (GpioDataRegs.GPDDAT.bit.GPIO97 && gui.press5) { loseKey = 6; gui.press5--; }
                    else if (GpioDataRegs.GPCDAT.bit.GPIO94 && gui.press6) { loseKey = 7; gui.press6--; }
                    else if (GpioDataRegs.GPCDAT.bit.GPIO65 && gui.press7) { loseKey = 8; gui.press7--; }
                    else if (GpioDataRegs.GPCDAT.bit.GPIO66 && gui.press8) { loseKey = 9; gui.press8--; }
                    else if (GpioDataRegs.GPCDAT.bit.GPIO67 && gui.press9) { loseKey = 10; gui.press9--; }
                    else if (GpioDataRegs.GPDDAT.bit.GPIO111 && gui.press10) { loseKey = 11; gui.press10--; }
                    else if (GpioDataRegs.GPBDAT.bit.GPIO32 && gui.press11) { loseKey = 12; gui.press11--; }
                    else if (GpioDataRegs.GPDDAT.bit.GPIO123 && gui.press12) { loseKey = 13; gui.press12--; }
                    else if (GpioDataRegs.GPBDAT.bit.GPIO59 && gui.press13) { loseKey = 14; gui.press13--; }

                    if (newKey || loseKey) {
                        gui.newBuff[gui.newIndex++] = 0x61;
                        gui.newBuff[gui.newIndex++] = (gui.waitTotal-gui.wait) & 0xFF;
                        gui.newBuff[gui.newIndex++] = ((gui.waitTotal-gui.wait) << 8) & 0xFF;
                        gui.waitTotal = gui.wait;

                        if (loseKey > 0 && loseKey < 13) {
                            gui.recPlaying = false;
                            switch(gui.recordLoop) {
                            case 1:
                                writeRegister(gui.apu, 0x03, 0x00);
                                gui.newBuff[gui.newIndex++] = 0xB4;
                                gui.newBuff[gui.newIndex++] = 0x03;
                                gui.newBuff[gui.newIndex++] = 0x00;
                                break;
                            case 2:
                                writeRegister(gui.apu, 0x07, 0x00);
                                gui.newBuff[gui.newIndex++] = 0xB4;
                                gui.newBuff[gui.newIndex++] = 0x07;
                                gui.newBuff[gui.newIndex++] = 0x00;
                                break;
                            case 3:
                                writeRegister(gui.apu, 0x0B, 0x00);
                                gui.newBuff[gui.newIndex++] = 0xB4;
                                gui.newBuff[gui.newIndex++] = 0x0B;
                                gui.newBuff[gui.newIndex++] = 0x00;
                                break;
                            case 4:
                                writeRegister(gui.apu, 0x0F, 0x00);
                                gui.newBuff[gui.newIndex++] = 0xB4;
                                gui.newBuff[gui.newIndex++] = 0x0F;
                                gui.newBuff[gui.newIndex++] = 0x00;
                                break;
                            }
                        }

                        if (newKey) {
                            if (newKey == 13) {
                                // OCTAVE UP
                                if (gui.octave < 5) {
                                    gui.octave++;
                                }
                            }
                            else if (newKey == 14) {
                                //OCTAVE DOWN
                                if (gui.octave > 0) {
                                    gui.octave--;
                                }
                            }
                            else {
                                int16 period = periodTable[gui.octave][newKey-1];
                                gui.recPlaying = true;
                                switch(gui.recordLoop) {
                                case 1:
                                    writeRegister(gui.apu, 0x00, gui.pu1.reg0);
                                    writeRegister(gui.apu, 0x01, gui.pu1.reg1);
                                    writeRegister(gui.apu, 0x02, period & 0xFF);
                                    writeRegister(gui.apu, 0x03, 0xF1 | ((period >> 8) & 0x7));
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x00;
                                    gui.newBuff[gui.newIndex++] = gui.pu1.reg0;
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x01;
                                    gui.newBuff[gui.newIndex++] = gui.pu1.reg1;
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x02;
                                    gui.newBuff[gui.newIndex++] = period & 0xFF;
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x03;
                                    gui.newBuff[gui.newIndex++] = 0xF1 | ((period >> 8) & 0x7);
                                    break;
                                case 2:
                                    writeRegister(gui.apu, 0x04, gui.pu2.reg0);
                                    writeRegister(gui.apu, 0x05, gui.pu2.reg1);
                                    writeRegister(gui.apu, 0x06, period & 0xFF);
                                    writeRegister(gui.apu, 0x07, 0xF1 | ((period >> 8) & 0x7));
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x04;
                                    gui.newBuff[gui.newIndex++] = gui.pu2.reg0;
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x05;
                                    gui.newBuff[gui.newIndex++] = gui.pu2.reg1;
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x06;
                                    gui.newBuff[gui.newIndex++] = period & 0xFF;
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x07;
                                    gui.newBuff[gui.newIndex++] = 0xF1 | ((period >> 8) & 0x7);
                                    break;
                                case 3:
                                    writeRegister(gui.apu, 0x08, gui.tri.reg0);
                                    writeRegister(gui.apu, 0x0A, period & 0xFF);
                                    writeRegister(gui.apu, 0x0B, 0xF1 | ((period >> 8) & 0x7));
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x08;
                                    gui.newBuff[gui.newIndex++] = gui.tri.reg0;
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x0A;
                                    gui.newBuff[gui.newIndex++] = period & 0xFF;
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x0B;
                                    gui.newBuff[gui.newIndex++] = 0xF1 | ((period >> 8) & 0x7);
                                    break;
                                case 4:
                                    writeRegister(gui.apu, 0x0C, gui.noi.reg0);
                                    writeRegister(gui.apu, 0x0E, period & 0xFF);
                                    writeRegister(gui.apu, 0x0F, 0xF1 | ((period >> 8) & 0x7));
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x0C;
                                    gui.newBuff[gui.newIndex++] = gui.noi.reg0;
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x0E;
                                    gui.newBuff[gui.newIndex++] = period & 0xFF;
                                    gui.newBuff[gui.newIndex++] = 0xB4;
                                    gui.newBuff[gui.newIndex++] = 0x0F;
                                    gui.newBuff[gui.newIndex++] = 0xF1 | ((period >> 8) & 0x7);
                                    break;
                                }
                            }
                        }
                    }
                    sample = processCycles(&apu, cyclesToProcess);
                    cyclesToProcess = 0;
                }
            }
        }

        else {
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
        cyclesToProcess = cyclesPerSample;
    }
    // dummy read
    Uint16 lo = McbspbRegs.DRR1.all;
    Uint16 hi = McbspbRegs.DRR2.all;
    // output the current sample
    McbspbRegs.DXR1.all = sample & 0xFFFF;
    McbspbRegs.DXR2.all = (sample >> 16) & 0xFFFF;
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
