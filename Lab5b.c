#include <F28x_Project.h>
#include "interrupt.h"
#include "InitAIC23.h"

interrupt void mcBspRx();

int main() {
    InitSysCtrl();
    InitBigBangedCodecSPI();
    InitAIC23();
    InitMcBSPb();
    Interrupt_initModule();
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_enableMaster();
}

interrupt void mcBspRx() {
    McbspbRegs.DXR1.all = McbspbRegs.DRR1.all;
    McbspbRegs.DXR2.all = McbspbRegs.DRR2.all;
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
