#include <F28x_Project.h>
#include <math.h>
#include "adc.h"
#include "cputimer.h"
#include "interrupt.h"
#include "lcd.h"

interrupt void TIMER1_INT();

volatile Uint16 prevVal;

int main(void) {
    InitSysCtrl();
    setup_lcd();
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_4_0);
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);
    ADC_enableConverter(ADCA_BASE);
    DELAY_US(1000);
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_SW_ONLY, ADC_CH_ADCIN0, 15);
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer1, 200, 100000);
    Interrupt_initModule();
    Interrupt_enable(INT_TIMER1);
    Interrupt_register(INT_TIMER1, &TIMER1_INT);
    Interrupt_enableMaster();
    CPUTimer_startTimer(CPUTIMER1_BASE);
    ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER0);
}

interrupt void TIMER1_INT() {
    Uint16 result =  ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
    float voltage_float = (300.0/4096.0)*result;
    Uint16 voltage = round(voltage_float);
    if(voltage != prevVal) {
        prevVal = voltage;
        char string[] = "Voltage = 0.00V";
        string[10] = voltage/100 % 10 + '0';
        string[12] = voltage/10 % 10 + '0';
        string[13] = voltage % 10 + '0';
        clear_screen();
        write_string(string);
    }
    ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER0);
    Interrupt_clearACKGroup(INTERRUPT_CPU_INT1);
}
