#include <F28x_Project.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "sram.h"
#include "io.h"

interrupt void mcBspRx();

volatile Uint16 isLeft = 1;
volatile int16 leftVal = 0;
volatile int16 oldVal = 0;
volatile Uint32 addr = 0;
volatile Uint16 bufferFull;
float echoVolume = 0.4;
Uint16 delaySamples = 0;

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
        Uint16 factor = (GpioDataRegs.GPADAT.all & 0x00000F00) >> 8;
        float samplesFloat = 46875 * 0.25 * factor;
        delaySamples = (Uint16) samplesFloat;
    }

}

interrupt void mcBspRx() {
    volatile int16 dummy = McbspbRegs.DRR1.all;
    int16 value = McbspbRegs.DRR2.all;
    if(isLeft) {
        isLeft = 0;
        leftVal = value;
        if(addr < delaySamples && !bufferFull) {
            oldVal = 0;
        }
        else {
            oldVal = read_sram((addr-delaySamples)%262144);
        }
        if(addr >= delaySamples) {
            bufferFull = true;
        }
    }
    else {
        isLeft = 1;
        int32 monoValBig = (leftVal + value)/2;
        int16 monoVal = (int16) monoValBig;
        float echoValFloat = monoVal * (1-echoVolume) + oldVal * echoVolume;
        int16 echoVal = (int16) echoValFloat;
        write_sram(addr, echoVal);
        McbspbRegs.DXR1.all = 0;
        McbspbRegs.DXR2.all = echoVal;
        addr = (addr+1)%262144;
    }
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
