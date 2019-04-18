#include <F28x_Project.h>
#include "interrupt.h"
#include "InitAIC23.h"
#include "adc.h"
#include "cputimer.h"

interrupt void mcBspRx();
interrupt void TIMER1_INT();
float volume;

int main() {
    InitSysCtrl();

    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_enableConverter(ADCA_BASE);
    DELAY_US(1000);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN0, 15);
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer1, 200, 100000);

    InitBigBangedCodecSPI();
    InitAIC23();
    InitMcBSPb();

    CPUTimer_startTimer(CPUTIMER1_BASE);
    Interrupt_initModule();
    Interrupt_enable(INT_MCBSPB_RX);
    Interrupt_enable(INT_TIMER1);
    Interrupt_register(INT_MCBSPB_RX, &mcBspRx);
    Interrupt_register(INT_TIMER1, &TIMER1_INT);
    Interrupt_enableMaster();
    ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER0);
}

interrupt void mcBspRx() {
    Uint32 lsb = McbspbRegs.DRR1.all;
    Uint32 msb = McbspbRegs.DRR2.all;
    Uint32 msb16 = (msb << 16);
    int32 val = msb16 | lsb;
    int32 scaled = val * volume;
    Uint16 outmsb = (scaled>>16) & 0xFFFF;
    Uint16 outlsb = scaled & 0xFFFF;
    McbspbRegs.DXR2.all = outmsb;
    McbspbRegs.DXR1.all = outlsb;
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);
}

interrupt void TIMER1_INT() {
    Uint16 result =  ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
    volume = (1.0/4096.0)*result;
    ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER0);
    Interrupt_clearACKGroup(INTERRUPT_CPU_INT1);
}
