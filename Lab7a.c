#include <F28x_Project.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "io.h"

interrupt void mcBspRx();

//Tap Coeffs from TFILTER, << 16
#define LOW_PASS_TAPS 125
int16 low_pass[LOW_PASS_TAPS] = {
 -2, -1, 0, 0, 2, 3, 6, 8, 11, 14, 17,
 19, 19, 18, 14, 8, 0, -11, -23, -36,
 -49, -61, -69, -73, -70, -61, -45, -21,
 10, 46, 84, 123, 157, 184, 199, 199,
 182, 146, 90, 17, -71, -168, -267, -360,
 -438, -492, -513, -492, -424, -305, -132,
 91, 360, 666, 998, 1343, 1686, 2010, 2302,
 2545, 2728, 2842, 2880, 2842, 2728, 2545,
 2302, 2010, 1686, 1343, 998, 666, 360,
 91, -132, -305, -424, -492, -513, -492,
 -438, -360, -267, -168, -71, 17, 90, 146,
 182, 199, 199, 184, 157, 123, 84, 46, 10,
 -21, -45, -61, -70, -73, -69, -61, -49,
 -36, -23, -11, 0, 8, 14, 18, 19, 19, 17,
 14, 11, 8, 6, 3, 2, 0, 0, -1, -2
};

//Tap Coeffs from TFILTER, << 16
#define HIGH_PASS_TAPS 33
int16 high_pass[HIGH_PASS_TAPS] = {
  -1043, 210, 320, 472, 620, 713, 698, 534, 198, -311,
  -967, -1719, -2494, -3215, -3800, -4181, 28455,
  -4181, -3800, -3215, -2494, -1719, -967, -311,
  198, 534, 698, 713, 620, 472, 320, 210, -1043
};

//Since Global, Buffer Init to 0;
#define MASK 0xFF
int16 buffer[MASK+1];
Uint16 index;

volatile Uint16 isLeft = 1;
volatile int16 halfValLeft = 0;
volatile int16 val = 0;
volatile Uint16 go = 0;

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

    while(1) {
        if(go) {

            //insert into buffer
            buffer[index++ & MASK] = val;

            //apply filter
            int32 acc = 0;
            if(GpioDataRegs.GPADAT.bit.GPIO11) { //choose which filter
                for(int i = 0; i < LOW_PASS_TAPS; i++) {
                    acc += ((int32) buffer[(index-i) & MASK]) * low_pass[i];
                }
            }
            else {
                for(int i = 0; i < HIGH_PASS_TAPS; i++) {
                    acc += ((int32) buffer[(index-i) & MASK]) * high_pass[i];
                }
            }

            //output
            McbspbRegs.DXR1.all = acc & 0xFFFF;
            McbspbRegs.DXR2.all = (acc >> 16) & 0xFFFF;

            go = 0;
        }
    }
}

interrupt void mcBspRx() {
    volatile int16 dummy = McbspbRegs.DRR1.all;
    int16 halfVal = ((int16) McbspbRegs.DRR2.all) >> 1;
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
