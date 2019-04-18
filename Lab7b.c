#include <filter.h>
#include <F28x_Project.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "bandpass.h"
#include "io.h"

interrupt void mcBspRx();

volatile Uint16 isLeft = 1;
volatile int32 halfValLeft = 0;
volatile int32 val = 0;
volatile Uint16 go = 0;

IIR5BIQ32  iir = IIR5BIQ32_DEFAULTS;
int32 dbuffer[2*IIR32_NBIQ];
const int32 coeff[5*IIR32_NBIQ] = IIR32_COEFF;

int main() {

    //Init DSP and Interrupt
    InitSysCtrl();
    init_io_gpio();
    InitBigBangedCodecSPI();
    InitAIC23(SR48);
    InitMcBSPb();
    Interrupt_initModule();
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_enableMaster();

    //init iir
    iir.dbuffer_ptr = dbuffer;
    iir.coeff_ptr   = (int32 *)coeff;
    iir.qfmat        = IIR32_QFMAT;
    iir.nbiq         = IIR32_NBIQ;
    iir.isf          = IIR32_ISF;
    iir.init(&iir);

    while(1) {
        if(go) {

            int32 filteredVal;
            if(GpioDataRegs.GPADAT.bit.GPIO10) { //decide if should filter
                //filter
                iir.input = val;
                iir.calc(&iir);
                filteredVal = iir.output32;
            }
            else {
                filteredVal = val;
            }

            //output
            McbspbRegs.DXR1.all = filteredVal & 0xFFFF;
            McbspbRegs.DXR2.all = (filteredVal >> 16) & 0xFFFF;

            go = 0;
        }
    }
}

interrupt void mcBspRx() {
    Uint16 lo = McbspbRegs.DRR1.all;
    Uint16 hi = McbspbRegs.DRR2.all;
    int32 halfVal = ( ( ((int32) hi) << 16) | lo ) >> 1;
    if(isLeft) {
        isLeft = 0;
        halfValLeft = halfVal;
    }
    else {
        isLeft = 1;
        val = halfValLeft + halfVal;
        go = 1;
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
