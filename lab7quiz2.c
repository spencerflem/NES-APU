#include <F28x_Project.h>
#include <filter.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "quiz2.h"
#include "AIC23.h"
#include "io.h"


interrupt void mcBspRx();

//Tap Coeffs from TFILTER, << 16
#define LOW_PASS_TAPS 33
float low_pass[LOW_PASS_TAPS] = {
    -0.007520285901, 0.001187347458, 0.002948948648, 0.005905866623,  0.01008212846,
    0.0154476827,  0.02192489244,  0.02933927625,  0.03745606914,   0.0459779501,
    0.05451973155,  0.06268507242,   0.0700699538,  0.07628035545,  0.08098600805,
    0.0839169398,  0.08491259068,   0.0839169398,  0.08098600805,  0.07628035545,
    0.0700699538,  0.06268507242,  0.05451973155,   0.0459779501,  0.03745606914,
    0.02933927625,  0.02192489244,   0.0154476827,  0.01008212846, 0.005905866623,
    0.002948948648, 0.001187347458,-0.007520285901
};

//Tap Coeffs from TFILTER, << 16
#define HIGH_PASS_TAPS 29
double high_pass[HIGH_PASS_TAPS] = {
    -0.05646257520393,  0.02455797370795,  0.02503865427885,  0.02649666521383,
    0.02641574491144,  0.02246591885989,  0.01310354219705,-0.002368472429728,
    -0.0234684629071, -0.04860693243296, -0.07531319143535,  -0.1007492678674,
    -0.1216365374204,  -0.1353537836901,   0.8598150690846,  -0.1353537836901,
    -0.1216365374204,  -0.1007492678674, -0.07531319143535, -0.04860693243296,
    -0.0234684629071,-0.002368472429728,  0.01310354219705,  0.02246591885989,
    0.02641574491144,  0.02649666521383,  0.02503865427885,  0.02455797370795,
    -0.05646257520393
};

//Since Global, Buffer Init to 0;
#define MASK 0xFF
int32 buffer[MASK+1];
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
            buffer[index++ & MASK] = val;
            iir.input = val;
            iir.calc(&iir);

            Uint16 mode = GpioDataRegs.GPADAT.bit.GPIO9<<1 | GpioDataRegs.GPADAT.bit.GPIO8;
            int32 filteredVal = 0;
            double filFlt = 0;

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
                    filFlt += buffer[(index-i) & MASK] * low_pass[i];
                }
                filteredVal = filFlt*9.44;
                break;
            case 3:
                // HIGH PASS FILTER
                for(int i = 0; i < HIGH_PASS_TAPS; i++) {
                    filFlt += buffer[(index-i) & MASK] * high_pass[i];
                }
                filteredVal = filFlt*11.02;
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
    int32 halfVal = ( ( ((int32) hi) << 16) | lo );
    if(isLeft) {
        isLeft = 0;
        halfValLeft = halfVal;
    }
    else {
        isLeft = 1;
        //val = halfValLeft + halfVal;
        val = halfVal;// << 1; //for making BODE PLOT
        go = 1;
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
