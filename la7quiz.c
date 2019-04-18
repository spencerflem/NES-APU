#include <F28x_Project.h>
#include <filter.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "quiz2.h"
#include "AIC23.h"
#include "io.h"


interrupt void mcBspRx();

//Tap Coeffs from TFILTER, << 16
#define LOW_PASS_TAPS 81
int16 low_pass[LOW_PASS_TAPS] = {  -22,
                                   -12,
                                   -12,
                                   -9,
                                   -3,
                                   7,
                                   22,
                                   41,
                                   63,
                                   87,
                                   110,
                                   130,
                                   143,
                                   147,
                                   138,
                                   115,
                                   75,
                                   19,
                                   -52,
                                   -133,
                                   -220,
                                   -306,
                                   -382,
                                   -438,
                                   -467,
                                   -458,
                                   -403,
                                   -299,
                                   -141,
                                   70,
                                   330,
                                   631,
                                   963,
                                   1312,
                                   1661,
                                   1996,
                                   2297,
                                   2551,
                                   2742,
                                   2862,
                                   2902,
                                   2862,
                                   2742,
                                   2551,
                                   2297,
                                   1996,
                                   1661,
                                   1312,
                                   963,
                                   631,
                                   330,
                                   70,
                                   -141,
                                   -299,
                                   -403,
                                   -458,
                                   -467,
                                   -438,
                                   -382,
                                   -306,
                                   -220,
                                   -133,
                                   -52,
                                   19,
                                   75,
                                   115,
                                   138,
                                   147,
                                   143,
                                   130,
                                   110,
                                   87,
                                   63,
                                   41,
                                   22,
                                   7,
                                   -3,
                                   -9,
                                   -12,
                                   -12,
                                   -22
};

//Tap Coeffs from TFILTER, << 16
#define HIGH_PASS_TAPS 29
int16 high_pass[HIGH_PASS_TAPS] = {
                                   -1761,
                                   805,
                                   815,
                                   858,
                                   851,
                                   718,
                                   410,
                                   -96,
                                   -783,
                                   -1602,
                                   -2471,
                                   -3294,
                                   -3971,
                                   -4416,
                                   28196,
                                   -4416,
                                   -3971,
                                   -3294,
                                   -2471,
                                   -1602,
                                   -783,
                                   -96,
                                   410,
                                   718,
                                   851,
                                   858,
                                   815,
                                   805,
                                   -1761
};

//Since Global, Buffer Init to 0;
#define MASK 0xFF
int16 buffer[MASK+1];
Uint16 index;

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

    //init iir
    iir.dbuffer_ptr = dbuffer;
    iir.coeff_ptr   = (int32 *)coeff;
    iir.qfmat        = IIR32_QFMAT;
    iir.nbiq         = IIR32_NBIQ;
    iir.isf          = IIR32_ISF;
    iir.init(&iir);

    while(1) {
        if(go) {

            // Insert Into Buffers
            buffer[index++ & MASK] = val>>16 & 0xFFFF;
            iir.input = val;
            iir.calc(&iir);

            Uint16 mode = GpioDataRegs.GPADAT.bit.GPIO9<<1 | GpioDataRegs.GPADAT.bit.GPIO8;
            int32 filteredVal = 0;

            // Filter
            EALLOW;
            GpioDataRegs.GPCTOGGLE.bit.GPIO95 = 1;
            EDIS;
            switch(mode) {
            case 0:
                // ALL PASS FILTER
                filteredVal = val;
                break;
            case 1:
                // BAND PASS FILTER
                filteredVal = iir.output32 * 23;
                break;
            case 2:
                // LOW PASS FILTER
                for(int i = 0; i < LOW_PASS_TAPS; i++) {
                    filteredVal += ((int32) buffer[(index-i) & MASK]) * low_pass[i];
                }
                filteredVal *= 23;
                break;
            case 3:
                // HIGH PASS FILTER
                for(int i = 0; i < HIGH_PASS_TAPS; i++) {
                    filteredVal += ((int32) buffer[(index-i) & MASK]) * high_pass[i];
                }
                filteredVal *= 23;
                break;
            }
            EALLOW;
            GpioDataRegs.GPCTOGGLE.bit.GPIO95 = 1;
            EDIS;

            // Output
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
        //val = halfValLeft + halfVal;
        val = halfVal << 1; //for making BODE PLOT
        go = 1;
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
