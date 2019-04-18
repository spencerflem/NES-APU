#include <F28x_Project.h>
#include <math.h>
#include <stdbool.h>
#include "adc.h"
#include "cputimer.h"
#include "interrupt.h"
#include "lcd.h"
#include "InitAIC23.h"
#include "AIC23.h"
#include "APU.h"

interrupt void TIMER1_INT();
interrupt void mcBspRx();

volatile float cyclesToProcess = 0;
int32 index = 0;
int16 wait = 0;
int32 sample = 0;

int main(void) {

    // inits
    InitSysCtrl();
    InitBigBangedCodecSPI();
    InitAIC23(SR48);
    InitMcBSPb();
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer1, 200, 1000000.0f/samplesPerSecond);
    Interrupt_initModule();
    Interrupt_enable(INT_TIMER1);
    Interrupt_register(INT_TIMER1, &TIMER1_INT);
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_enableMaster();
    CPUTimer_startTimer(CPUTIMER1_BASE); //test

    // Init APU (have function to set up values!!)
    struct Apu apu = {};
    initApu(&apu);

    while (1) {
        // Next Sample:
        if (cyclesToProcess > 0) {
            if (wait == 0) {
                while (wait == 0) {
                    Uint16 command = file[index];
                }
            }
            else {
                wait--;
            }
            sample = processCycles(&apu, cyclesToProcess);
            cyclesToProcess = 0;
        }
    }

}

interrupt void TIMER1_INT() {
    // start processing next sample
    cyclesToProcess += cyclesPerSample;
    Interrupt_clearACKGroup(INTERRUPT_CPU_INT1);
}

interrupt void mcBspRx() {
    // dummy read
    Uint16 lo = McbspbRegs.DRR1.all;
    Uint16 hi = McbspbRegs.DRR2.all;
    // output the current sample
    McbspbRegs.DXR1.all = sample & 0xFFFF;
    McbspbRegs.DXR2.all = (sample >> 16) & 0xFFFF;
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}
