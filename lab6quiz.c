#include <F28x_Project.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "sram.h"
#include "io.h"

#define a 0.2f
#define p 480
#define q 1000
#define r 1500

interrupt void mcBspRx();

volatile Uint16 isLeft = 1;
volatile int16 leftVal = 0;
volatile int16 oldVal1 = 0;
volatile int16 oldVal2 = 0;
volatile int16 oldVal3 = 0;
volatile Uint32 addr = 0;
volatile Uint16 bufferFull;
float reverbVolume = 0.4;
Uint16 delaySamples = 0;
Uint16 want = 0;
volatile Uint16 go = 0;
volatile int16 val = 0;

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
        if(go) {
            if(bufferFull) {
                oldVal1 = read_sram((addr-p)%262144);
                oldVal2 = read_sram((addr-q)%262144);
                oldVal3 = read_sram((addr-r)%262144);
            }
            else if(addr >= r) {
                bufferFull = 1;
            }
            write_sram(addr, val);
            float reverbValFloat = val * (1-3*a) + oldVal1 * a + oldVal2 * a + oldVal3 * a;
            int16 reverbVal = (int16) reverbValFloat;
            McbspbRegs.DXR1.all = 0;
            McbspbRegs.DXR2.all = reverbVal;
            addr = (addr+1)%262144;
            go = 0;
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
        go = 1;
        int32 monoValBig = (leftVal + value)/2;
        val = (int16) monoValBig;
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
