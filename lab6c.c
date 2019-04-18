#include <F28x_Project.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "sram.h"
#include "io.h"

interrupt void mcBspRx();

volatile Uint16 isLeft = 1;
volatile int16 leftVal = 0;
volatile Uint32 addr = 0;
enum State{Idle, Recording, Mixing, Playback};
volatile enum State state = Idle;
enum Mode{Normal, Interp, Decim};
enum Mode mode;
volatile int16 prevVal = 0;
volatile int16 currentVal = 0;
volatile Uint16 decimCount = 0;
Uint16 prevRate = 48;
Uint16 currentRate = 48;

int main() {
    InitSysCtrl();
    init_sram_spi();
    init_io_gpio();
    InitBigBangedCodecSPI();
    InitAIC23(SR48);
    InitMcBSPb();
    Interrupt_initModule();
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_enableMaster();

    while(1) {
        if(state == Idle) {
            //state
            if(!GpioDataRegs.GPADAT.bit.GPIO16) {
                GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;
                state = Recording;
            }
            else if(!GpioDataRegs.GPADAT.bit.GPIO15) {
                GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;
                state = Mixing;
            }
            else if(!GpioDataRegs.GPADAT.bit.GPIO14) {
                GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;
                state = Playback;
            }
            //rate
            if(GpioDataRegs.GPADAT.bit.GPIO9) {
               currentRate = 48;
            }
            else if(GpioDataRegs.GPADAT.bit.GPIO8) {
                currentRate = 32;
            }
            else {
                currentRate = 8;
            }
            if(currentRate != prevRate)
            {
                prevRate = currentRate;
                switch(currentRate) {
                case 48:
                    InitAIC23(SR48);
                    break;
                case 32:
                    InitAIC23(SR32);
                    break;
                case 8:
                    InitAIC23(SR8);
                    break;
                }
            }
            //mode
            if(!GpioDataRegs.GPADAT.bit.GPIO11) {
                mode = Normal;
            }
            else if(GpioDataRegs.GPADAT.bit.GPIO10) {
                mode = Interp;
            }
            else {
                mode = Decim;
            }
        }
    }

}

interrupt void mcBspRx() {
    volatile int16 dummy = McbspbRegs.DRR1.all;
    int16 value = McbspbRegs.DRR2.all;
    if(isLeft) {
        isLeft = 0;
        leftVal = value;
        if(mode == Interp && state == Recording) {
            if(addr > 1) {
                int32 interpValBig = (prevVal + currentVal)/2;
                int16 interpVal = (int16) interpValBig;
                write_sram(addr-1, interpVal);
            }
            prevVal = currentVal;
        }
    }
    else {
        isLeft = 1;
        if(addr < 262144) {
            switch(state) {
                case Recording: {
                    int32 monoValBig = (leftVal + value)/2;
                    int16 monoVal = (int16) monoValBig;
                    switch(mode) {
                    case Normal: {
                        write_sram(addr++, monoVal);
                        break;
                    }
                    case Interp: {
                        write_sram(addr++, monoVal);
                        addr++;
                        currentVal = monoVal;
                        break;
                    }
                    case Decim: {
                        if(decimCount == 0) {
                            write_sram(addr++, monoVal);
                        }
                        decimCount = (decimCount+1)%5;
                        break;
                    }
                    }
                    break;
                }
                case Mixing: {
                    int32 monoVal = (leftVal + value)/2;
                    int16 oldVal = read_sram(addr);
                    int32 mixValBig = (monoVal + oldVal)/2;
                    int16 mixVal = (int16) mixValBig;
                    write_sram(addr++, mixVal);
                    break;
                }
                case Playback: {
                    int16 oldVal = read_sram(addr++);
                    McbspbRegs.DXR1.all = 0;
                    McbspbRegs.DXR2.all = oldVal;
                    break;
                }
            }
        }
        else {
            addr = 0;
            GpioDataRegs.GPASET.all = 0x00000007; //clear leds
            state = Idle;
        }
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
