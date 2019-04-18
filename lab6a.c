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
        }
    }

}

interrupt void mcBspRx() {
    volatile int16 dummy = McbspbRegs.DRR1.all;
    int16 value = McbspbRegs.DRR2.all;
    if(isLeft) {
        isLeft = 0;
        leftVal = value;
    }
    else {
        isLeft = 1;
        if(addr < 262144) {
            switch(state) {
                case Recording: {
                    int32 monoValBig = (leftVal + value)/2;
                    int16 monoVal = (int16) monoValBig;
                    write_sram(addr++, monoVal);
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
