#include <F28x_Project.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "sram.h"
#include "io.h"

interrupt void mcBspRx();

Uint16 prevRate = 48;
Uint16 currentRate = 48;

int main() {
    InitSysCtrl();
    init_sram_spi();
    init_io_gpio();
    EALLOW;
    GpioCtrlRegs.GPCMUX2.bit.GPIO95 = 0;
    GpioCtrlRegs.GPCDIR.bit.GPIO95 = 1;
    EDIS;
    InitBigBangedCodecSPI();
    InitAIC23(SR48);
    InitMcBSPb();
    Interrupt_initModule();
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_enableMaster();

    while(1) {
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
    }

}

interrupt void mcBspRx() {
    volatile int16 dummy = McbspbRegs.DRR1.all;
    int16 value = McbspbRegs.DRR2.all;
    EALLOW;
    GpioDataRegs.GPCTOGGLE.bit.GPIO95 = 1;
    EDIS;
    McbspbRegs.DXR1.all = 0;
    McbspbRegs.DXR2.all = value;
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
